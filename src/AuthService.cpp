//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "Poco/Net/OAuth20Credentials.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"

#include "AuthService.h"
#include "Daemon.h"
#include "RESTAPI_handler.h"
#include "StorageService.h"
#include "Utils.h"

namespace uCentral {
    class AuthService *AuthService::instance_ = nullptr;

	AuthService::ACCESS_TYPE AuthService::IntToAccessType(int C) {
		switch (C) {
		case 1: return USERNAME;
		case 2: return SERVER;
		case 3: return CUSTOM;
		default:
			return USERNAME;
		}
	}

	int AuthService::AccessTypeToInt(ACCESS_TYPE T) {
		switch (T) {
		case USERNAME: return 1;
		case SERVER: return 2;
		case CUSTOM: return 3;
		}
		return 1;	// some compilers complain...
	}

	AuthService::AuthService() noexcept:
	  SubSystemServer("Authentication", "AUTH-SVR", "authentication")
    {
    }

    int AuthService::Start() {
		Signer_.setRSAKey(Daemon()->Key());
		Signer_.addAllAlgorithms();
		Logger_.notice("Starting...");
        Secure_ = Daemon()->ConfigGetBool(SubSystemConfigPrefix_+".enabled",true);
        DefaultPassword_ = Daemon()->ConfigGetString(SubSystemConfigPrefix_+".default.password","");
        DefaultUserName_ = Daemon()->ConfigGetString(SubSystemConfigPrefix_+".default.username","");
        Mechanism_ = Daemon()->ConfigGetString(SubSystemConfigPrefix_+".service.type","internal");
        return 0;
    }

    void AuthService::Stop() {
		Logger_.notice("Stopping...");
    }

	bool AuthService::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, struct uCentral::Objects::WebToken & UserInfo  )
    {
        if(!Secure_)
            return true;

		SubMutexGuard		Guard(Mutex_);

		std::string CallToken;

		try {
			Poco::Net::OAuth20Credentials Auth(Request);

			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}
		} catch(const Poco::Exception &E) {
		}

		if(CallToken.empty())
			CallToken = Request.get("X-API-KEY ", "");

		if(CallToken.empty())
			return false;

		auto Client = Tokens_.find(CallToken);

		if( Client == Tokens_.end() )
			return ValidateToken(CallToken, CallToken, UserInfo);

		if((Client->second.created_ + Client->second.expires_in_) > time(nullptr)) {
			SessionToken = CallToken;
			UserInfo = Client->second ;
			return true;
		}

		Tokens_.erase(CallToken);
		return false;
    }

    void AuthService::Logout(const std::string &token) {
		SubMutexGuard		Guard(Mutex_);
        Tokens_.erase(token);
    }

    std::string AuthService::GenerateToken(const std::string & Identity, ACCESS_TYPE Type, int NumberOfDays) {
		SubMutexGuard		Guard(Mutex_);

		Poco::JWT::Token	T;

		T.setType("JWT");
		switch(Type) {
			case USERNAME:	T.setSubject("usertoken"); break;
			case SERVER: 	T.setSubject("servertoken"); break;
			case CUSTOM:	T.setSubject("customtoken"); break;
		}

		T.payload().set("identity", Identity);
		T.setIssuedAt(Poco::Timestamp());
		T.setExpiration(Poco::Timestamp() + Poco::Timespan(NumberOfDays,0,0,0,0));
		std::string JWT = Signer_.sign(T,Poco::JWT::Signer::ALGO_RS256);

		return JWT;
    }

	bool AuthService::ValidateToken(const std::string & Token, std::string & SessionToken, struct uCentral::Objects::WebToken & UserInfo  ) {
		SubMutexGuard		Guard(Mutex_);
		Poco::JWT::Token	DecryptedToken;

		try {
			if (Signer_.tryVerify(Token, DecryptedToken)) {
				auto Expires = DecryptedToken.getExpiration();
				if (Expires > Poco::Timestamp()) {
					auto Identity = DecryptedToken.payload().get("identity").toString();
					auto IssuedAt = DecryptedToken.getIssuedAt();
					auto Subject = DecryptedToken.getSubject();

					UserInfo.access_token_ = Token;
					UserInfo.refresh_token_= Token;
					UserInfo.username_ = Identity;
					UserInfo.id_token_ = Token;
					UserInfo.token_type_ = "Bearer";
					UserInfo.created_ = IssuedAt.epochTime();
					UserInfo.expires_in_ = Expires.epochTime() - IssuedAt.epochTime();
					UserInfo.idle_timeout_ = 5*60;

					if(Storage()->GetIdentityRights(Identity, UserInfo.acl_template_)) {
					} else {
						//	we can get in but we have no given rights... something is very wrong
						UserInfo.acl_template_.Read_ = true ;
						UserInfo.acl_template_.ReadWriteCreate_ =
						UserInfo.acl_template_.ReadWrite_ =
						UserInfo.acl_template_.Delete_ = false;
						UserInfo.acl_template_.PortalLogin_ = true;
					}

					Tokens_[UserInfo.access_token_] = UserInfo;

					return true;
				}
			}
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
	}

    void AuthService::CreateToken(const std::string & UserName, uCentral::Objects::WebToken & UserInfo, uCentral::Objects::AclTemplate & ACL)
    {
		SubMutexGuard		Guard(Mutex_);

		std::string Token = GenerateToken(UserName,USERNAME,30);

		UserInfo.acl_template_ = ACL;

		UserInfo.expires_in_ = 30 * 24 * 60 * 60 ;
		UserInfo.idle_timeout_ = 5 * 60;
		UserInfo.token_type_ = "Bearer";
		UserInfo.access_token_ = Token;
		UserInfo.id_token_ = Token;
		UserInfo.refresh_token_ = Token;
		UserInfo.created_ = time(nullptr);
		UserInfo.username_ = UserName;

        Tokens_[UserInfo.access_token_] = UserInfo;
    }

    bool AuthService::Authorize( const std::string & UserName, const std::string & Password, uCentral::Objects::WebToken & ResultToken )
    {
		SubMutexGuard					Guard(Mutex_);
		uCentral::Objects::AclTemplate	ACL;

		if(Mechanism_=="internal")
        {
            if(((UserName == DefaultUserName_) && (Password == DefaultPassword_)) || !Secure_)
            {
				ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
                CreateToken(UserName, ResultToken, ACL);
                return true;
            }
        } else if (Mechanism_=="db") {
			SHA2_.update(Password + UserName);
			auto EncryptedPassword = Utils::ToHex(SHA2_.digest());

			std::string TUser{UserName};
			if(Storage()->GetIdentity(TUser,EncryptedPassword,USERNAME,ACL)) {
				CreateToken(UserName, ResultToken, ACL);
				return true;
			}
		}
        return false;
    }

}  // end of namespace