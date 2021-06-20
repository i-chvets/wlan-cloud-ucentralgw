//
// Created by stephane bourque on 2021-06-17.
//
#include <thread>
#include <fstream>
#include <vector>

#include "OUIServer.h"
#include "Daemon.h"

#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/File.h"

#include "Utils.h"

namespace uCentral {
	class OUIServer * OUIServer::instance_;

	int OUIServer::Start() {
		Running_=true;
		std::thread Updater([this]() { this->UpdateImpl(); });
		Updater.detach();
		return 0;
	}

	void OUIServer::Stop() {
		Running_=false;
	}

	void OUIServer::Update() {
		if(!Running_)
			return;
		std::thread Updater([this]() { this->UpdateImpl(); });
		Updater.detach();
	}

	static uint64_t MACtoInt(const std::string &MAC) {
		uint64_t Result = 0 ;
		int Digits=0;

		for(auto i:MAC) {
			if(std::isxdigit(i)) {
				if(i>='0' && i<='9') {
					Result <<=4;
					Result += i-'0';
				} else if(i>='A' && i<='F') {
					Result <<=4;
					Result += i-'A'+10;
				} else if(i>='a' && i<='f') {
					Result <<=4;
					Result += i-'a'+10;
				}
				Digits++;
				if(Digits==6)
					break;
			}
		}
		return Result;
	}

	bool OUIServer::GetFile(const std::string &FileName) {
		try {
			std::unique_ptr<std::istream> pStr(
				Poco::URIStreamOpener::defaultOpener().open(Daemon()->ConfigGetString("oui.download.uri")));
			std::ofstream OS;
			Poco::File	F(FileName);
			if(F.exists())
				F.remove();
			OS.open(FileName);
			Poco::StreamCopier::copyStream(*pStr, OS);
			OS.close();
			return true;

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool OUIServer::ProcessFile( const std::string &FileName, OUIMap &Map) {
		try {
			std::ifstream Input;
			Input.open(FileName, std::ios::binary);

			while (!Input.eof()) {
				char buf[256];
				Input.getline(buf, sizeof(buf));
				std::string Line{buf};
				auto Tokens = Poco::StringTokenizer(Line, " \t",
													Poco::StringTokenizer::TOK_TRIM |
														Poco::StringTokenizer::TOK_IGNORE_EMPTY);

				if (Tokens.count() > 2) {
					if (Tokens[1] == "(hex)") {
						auto MAC = MACtoInt(Tokens[0]);
						if (MAC > 0) {
							std::string Manufacturer;
							for (auto i = 2; i < Tokens.count(); i++)
								Manufacturer += Tokens[i] + " ";
							auto M = Poco::trim(Manufacturer);
							if (!M.empty())
								Map[MAC] = M;
						}
					}
				}
			}
			return true;
		} catch ( const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	void OUIServer::UpdateImpl() {
		if(Updating_)
			return;
		Updating_ = true;

		//	fetch data from server, if not available, just use the file we already have.
		std::string LatestOUIFileName{ Daemon()->DataDir() + "/newOUIFile.txt"};
		std::string CurrentOUIFileName{ Daemon()->DataDir() + "/current_oui.txt"};

		OUIMap TmpOUIs;
		if(GetFile(LatestOUIFileName) && ProcessFile(LatestOUIFileName, TmpOUIs)) {
			SubMutexGuard G(Mutex_);
			OUIs_ = std::move(TmpOUIs);
			LastUpdate_ = time(nullptr);
			Poco::File F1(CurrentOUIFileName);
			if(F1.exists())
				F1.remove();
			Poco::File F2(LatestOUIFileName);
			F2.renameTo(CurrentOUIFileName);
			Logger_.information(Poco::format("New OUI file %s downloaded.",LatestOUIFileName));
		} else if(OUIs_.empty()) {
			if(ProcessFile(CurrentOUIFileName, TmpOUIs)) {
				LastUpdate_ = time(nullptr);
				SubMutexGuard G(Mutex_);
				OUIs_ = std::move(TmpOUIs);
			}
		}
		Updating_ = false;
	}

	std::string OUIServer::GetManufacturer(const std::string &MAC) {
		SubMutexGuard Guard(Mutex_);
		auto Manufacturer = OUIs_.find(MACtoInt(MAC));
		if(Manufacturer != OUIs_.end())
			return Manufacturer->second;
		return "";
	}
};