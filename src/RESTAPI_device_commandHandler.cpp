//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/UUIDGenerator.h"
#include "Poco/JSON/Parser.h"

#include "RESTAPI_device_commandHandler.h"
#include "RESTAPI_objects.h"
#include "uCentral.h"
#include "uCentralConfig.h"
#include "uDeviceRegistry.h"
#include "uFileUploader.h"
#include "uStorageService.h"
#include "uUtils.h"

#include "uCentralProtocol.h"

void RESTAPI_device_commandHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    try {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        std::string Command = GetBinding("command", "");

        if (Command.empty()) {
            BadRequest(Request, Response);
            return;
        }

        ParseParameters(Request);

        if (Command == "capabilities" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetCapabilities(Request, Response);
        } else if (Command == "capabilities" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteCapabilities(Request, Response);
        } else if (Command == "logs" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetLogs(Request, Response);
        } else if (Command == "logs" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteLogs(Request, Response);
        } else if (Command == "healthchecks" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetChecks(Request, Response);
        } else if (Command == "healthchecks" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteChecks(Request, Response);
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatistics(Request, Response);
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteStatistics(Request, Response);
        } else if (Command == "status" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatus(Request, Response);
        } else if (Command == "perform" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            ExecuteCommand(Request, Response);
        } else if (Command == "configure" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Configure(Request, Response);
        } else if (Command == "upgrade" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Upgrade(Request, Response);
        } else if (Command == "reboot" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Reboot(Request, Response);
        } else if (Command == "factory" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Factory(Request, Response);
        } else if (Command == "leds" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            LEDs(Request, Response);
        } else if (Command == "trace" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Trace(Request, Response);
		} else if (Command == "request" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			MakeRequest(Request, Response);
		} else if (Command == "wifiscan" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			WifiScan(Request, Response);
		} else if (Command == "eventqueue" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			EventQueue(Request, Response);
		} else if (Command == "rtty" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			Rtty(Request, Response);
		} else {
            BadRequest(Request, Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    uCentral::Objects::Capabilities    Caps;
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        if (uCentral::Storage::GetDeviceCapabilities(SerialNumber, Caps)) {
            Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
            RetObj.set("serialNumber", SerialNumber);
            ReturnObject(Request, RetObj, Response );
        } else {
			NotFound(Request, Response);
		}
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        if (uCentral::Storage::DeleteDeviceCapabilities(SerialNumber))
            OK(Request, Response);
        else
            NotFound(Request, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);
		auto Lifetime = GetBoolParameter("lifetime",false);

		std::cout << __LINE__ << std::endl;
		if(Lifetime) {
			std::cout << __LINE__ << std::endl;
			std::string Stats;
			std::cout << __LINE__ << std::endl;
			std::cout << "STATS: " << Stats << std::endl;
			std::cout << __LINE__ << std::endl;
			std::cout << __LINE__ << std::endl;
			uCentral::Storage::GetLifetimeStats(SerialNumber,Stats);
			std::cout << __LINE__ << std::endl;
			std::cout << __LINE__ << std::endl;
			std::cout << "STATS: " << Stats << std::endl;
			std::cout << __LINE__ << std::endl;

			Poco::JSON::Parser	P;
			std::cout << __LINE__ << std::endl;
			if(Stats.empty())
				Stats = uCentral::uCentralProtocol::EMPTY_JSON_DOC;

			std::cout << __LINE__ << std::endl;
			auto Obj = P.parse(Stats).extract<Poco::JSON::Object>();
			ReturnObject(Request, Obj, Response);

		} else {
			std::vector<uCentral::Objects::Statistics> Stats;
			uCentral::Storage::GetStatisticsData(SerialNumber, StartDate, EndDate, Offset, Limit,
												 Stats);
			Poco::JSON::Array ArrayObj;
			for (auto i : Stats) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				ArrayObj.add(Obj);
			}
			Poco::JSON::Object RetObj;
			RetObj.set("data", ArrayObj);
			RetObj.set("serialNumber", SerialNumber);
			ReturnObject(Request, RetObj, Response);
		}

        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));
		auto Lifetime = GetBoolParameter("lifetime",false);

		if(Lifetime) {
			if(uCentral::Storage::ResetLifetimeStats(SerialNumber)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		} else {
			if (uCentral::Storage::DeleteStatisticsData(SerialNumber, StartDate, EndDate)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		}
		return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatus(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        uCentral::Objects::ConnectionState State;

        if (uCentral::DeviceRegistry::GetState(SerialNumber, State)) {

            Poco::JSON::Object RetObject;
			State.to_json(RetObject);

            ReturnObject(Request, RetObject, Response);

        } else {
			NotFound(Request, Response);
		}
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Configure(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        if(SNum.empty())
        {
            BadRequest(Request, Response);
            return;
        }

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("serialNumber") &&
            ds.contains("UUID") &&
            ds.contains("configuration")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum)
            {
                BadRequest(Request, Response);
                return;
            }

            auto UUID = ds["UUID"];
            auto Configuration = ds["configuration"].toString();
            uint64_t When = 0 ;

            if(ds.contains("when"))
                When = uCentral::Utils::from_RFC3339(ds["when"].toString());

            uint64_t NewUUID;

            if (uCentral::Storage::UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID)) {
                uCentral::Objects::CommandDetails  Cmd;

                Cmd.SerialNumber = SerialNumber;
                Cmd.UUID = uCentral::instance()->CreateUUID();
                Cmd.SubmittedBy = UserInfo_.username_;
                Cmd.Command = uCentral::uCentralProtocol::CONFIGURE;
                Cmd.Custom = 0;
                Cmd.RunAt = When;
                Cmd.WaitingForFile = 0;

                uCentral::Config::Config    Cfg(Configuration);

                Cfg.SetUUID(NewUUID);

                Poco::JSON::Object  Params;
				Poco::JSON::Object	CfgObj;

                Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber );
                Params.set(uCentral::uCentralProtocol::UUID, NewUUID);
                Params.set(uCentral::uCentralProtocol::WHEN, When);
				Cfg.to_json(CfgObj);
                Params.set(uCentral::uCentralProtocol::CONFIG, CfgObj);

                std::stringstream ParamStream;
                Params.stringify(ParamStream);
                Cmd.Details = ParamStream.str();

                if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd,Request, Response);
					return;
                } else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Upgrade(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("uri") &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            auto URI = ds["uri"].toString();

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = uCentral::Utils::from_RFC3339(ds["when"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Custom = 0;
            Cmd.Command = uCentral::uCentralProtocol::UPGRADE;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::URI, URI);
            Params.set( uCentral::uCentralProtocol::WHEN, When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);
        auto LogType = GetParameter("logType",0);

        std::vector<uCentral::Objects::DeviceLog> Logs;

        uCentral::Storage::GetLogData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                                                  Logs,LogType);
        Poco::JSON::Array ArrayObj;

        for (auto i : Logs) {
            Poco::JSON::Object Obj;
			i.to_json(Obj);
            ArrayObj.add(Obj);
        }
        Poco::JSON::Object RetObj;
        RetObj.set("values", ArrayObj);
        RetObj.set("serialNumber", SerialNumber);

        ReturnObject(Request, RetObj, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));
        auto LogType = GetParameter("logType",0);

        if (uCentral::Storage::DeleteLogData(SerialNumber, StartDate, EndDate, LogType)) {
			OK(Request, Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentral::Objects::HealthCheck> Checks;

        uCentral::Storage::GetHealthCheckData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                      Checks);
        Poco::JSON::Array ArrayObj;

        for (auto i : Checks) {
            Poco::JSON::Object Obj;
			i.to_json(Obj);
            ArrayObj.add(Obj);
        }
        Poco::JSON::Object RetObj;
        RetObj.set("values", ArrayObj);
        RetObj.set("serialNumber", SerialNumber);

        ReturnObject(Request, RetObj, Response);

        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteHealthCheckData(SerialNumber, StartDate, EndDate)) {
			OK(Request, Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::ExecuteCommand(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("command") &&
            ds.contains("serialNumber") &&
            ds.contains("payload")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            auto Command = ds["command"].toString();
            auto Payload = ds["payload"].toString();

            uint64_t RunAt = 0 ;
            if(ds.contains("runAt"))
                RunAt = uCentral::Utils::from_RFC3339(ds["runAt"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = Command;
            Cmd.Custom = 1;
            Cmd.RunAt = RunAt;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Parser parser2;

            Poco::Dynamic::Var result = parser2.parse(Payload);
            const auto & PayloadObject = result.extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::COMMAND, Command);
            Params.set( uCentral::uCentralProtocol::WHEN, RunAt);
            Params.set( uCentral::uCentralProtocol::PAYLOAD, PayloadObject);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Reboot(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct     ds = *Obj;

        if (ds.contains("serialNumber")) {
            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = uCentral::Utils::from_RFC3339(ds["when"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::REBOOT;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::WHEN, When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Factory(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("keepRedirector") &&
            ds.contains("serialNumber")) {

			auto SerialNumber = ds["serialNumber"].toString();

			if (SerialNumber != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto KeepRedirector = ds["keepRedirector"].toString();
			uint64_t KeepIt;
			if (KeepRedirector == "true")
				KeepIt = 1;
			else if (KeepRedirector == "false")
				KeepIt = 0;
			else {
				BadRequest(Request, Response);
				return;
			}

			uint64_t When = 0;
			if (ds.contains("when"))
				When = uCentral::Utils::from_RFC3339(ds["when"].toString());

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::FACTORY;
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
			Params.set(uCentral::uCentralProtocol::KEEP_REDIRECTOR, KeepIt);
			Params.set(uCentral::uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if (uCentral::Storage::AddCommand(SerialNumber, Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
			} else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::LEDs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::uCentralProtocol::PATTERN) &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

			auto Pattern = ds[uCentral::uCentralProtocol::PATTERN].toString();

			if(Pattern!=uCentral::uCentralProtocol::ON && Pattern!=uCentral::uCentralProtocol::OFF && Pattern!=uCentral::uCentralProtocol::BLINK)
			{
				Logger_.warning(Poco::format("LEDs(%s): Bad pattern",SerialNumber));
				BadRequest(Request, Response);
				return;
			}

			auto Duration = ds.contains(uCentral::uCentralProtocol::DURATION) ? (uint64_t ) ds[uCentral::uCentralProtocol::DURATION] : 20 ;

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = uCentral::Utils::from_RFC3339(ds["when"].toString());

			Logger_.information(Poco::format("LEDS(%s): Pattern:%s Duration: %d", SerialNumber, Pattern, (int)Duration));

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::LEDS;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set(uCentral::uCentralProtocol::DURATION, Duration);
            Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::PATTERN,Pattern);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Trace(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("serialNumber") && (
            	ds.contains("network") ||
            	ds.contains("interface"))) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            uint64_t Duration = ds.contains("duration") ? (uint64_t)ds["duration"] : 0;
            uint64_t When = ds.contains("when") ? uCentral::Utils::from_RFC3339(ds["when"].toString()) : 0;
            uint64_t NumberOfPackets = ds.contains("numberOfPackets") ? (uint64_t)ds["numberOfPackets"] : 0;

            auto Network = ds.contains("network") ? ds["network"].toString() : "";
            auto Interface = ds.contains("interface") ? ds["interface"].toString() : "";
            auto UUID = uCentral::instance()->CreateUUID();
            auto URI = uCentral::uFileUploader::FullName() + UUID ;

            uCentral::Objects::CommandDetails  Cmd;
            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = UUID;
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::TRACE;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 1;
			Cmd.AttachType = "trace";

            Poco::JSON::Object  Params;

            Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set(uCentral::uCentralProtocol::DURATION, Duration);
            Params.set(uCentral::uCentralProtocol::WHEN, When);
            Params.set(uCentral::uCentralProtocol::PACKETS, NumberOfPackets);
            Params.set(uCentral::uCentralProtocol::NETWORK, Network);
            Params.set(uCentral::uCentralProtocol::INTERFACE, Interface);
            Params.set(uCentral::uCentralProtocol::URI,URI);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
                uCentral::uFileUploader::AddUUID(UUID);
				Poco::JSON::Object RetObj;
				Cmd.to_json(RetObj);
				ReturnObject(Request, RetObj, Response);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::WifiScan(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try{
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains("serialNumber")) {
			auto SerialNumber = ds["serialNumber"].toString();
			if(	(ds.contains("bands") && ds["bands"].isArray()) ||
				(ds.contains("channels") && ds["channels"].isArray()) ||
				(!ds.contains("bands") && !ds.contains("channels"))
				)
			{
				bool Verbose = false ;

				if(ds.contains("verbose"))
				{
					Verbose = ds["verbose"].toString() == "true";
				}

				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::WIFISCAN;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
				Params.set(uCentral::uCentralProtocol::VERBOSE, Verbose);

				if( ds.contains(uCentral::uCentralProtocol::BANDS)) {
					Params.set(uCentral::uCentralProtocol::BANDS,ds["bands"]);
				} else if ( ds.contains(uCentral::uCentralProtocol::CHANNELS)) {
					Params.set(uCentral::uCentralProtocol::CHANNELS,ds["channels"]);
				}

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd, Request, Response, 20000);
					return;
				} else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			}
		}
	} catch (const Poco::Exception & E) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::EventQueue(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains("serialNumber") &&
			ds.contains("types") && ds["types"].isArray())
		{
			auto SerialNumber = ds["serialNumber"].toString();
			auto Types = ds["types"];

			if( SerialNumber == SNum ) {
				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::EVENT;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
				Params.set(uCentral::uCentralProtocol::TYPES, Types);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd, Request, Response, 20000);
					return;
				} else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			}
		}
	} catch ( const Poco::Exception & E ) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::MakeRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if (ds.contains("serialNumber") &&
			ds.contains("message")) {

			auto SerialNumber = ds["serialNumber"].toString();
			auto MessageType = ds["message"].toString();

			if ((SerialNumber != SNum) ||
				(MessageType != "state" && MessageType != "healthcheck")) {
				BadRequest(Request, Response);
				return;
			}

			uint64_t When =
				ds.contains("when") ? uCentral::Utils::from_RFC3339(ds["when"].toString()) : 0;

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.Command = uCentral::uCentralProtocol::REQUEST;
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::MESSAGE, MessageType);
			Params.set(uCentral::uCentralProtocol::REQUEST_UUID, Cmd.UUID);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if (uCentral::Storage::AddCommand(SerialNumber, Cmd)) {
				WaitForRPC(Cmd, Request, Response, 4000);
				return;
			} else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
		}
	}
	catch(const Poco::Exception &E)
	{
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Rtty(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {

		auto SerialNumber = GetBinding("serialNumber", "");

		if(uCentral::ServiceConfig::GetString("rtty.enabled","false") == "true") {

			if (uCentral::Storage::DeviceExists(SerialNumber)) {
				auto CommandUUID = uCentral::Daemon::instance()->CreateUUID();
				uCentral::Objects::RttySessionDetails Rtty{
					.SerialNumber = SerialNumber,
					.Server = uCentral::ServiceConfig::GetString("rtty.server", "localhost") ,
					.Port = uCentral::ServiceConfig::GetInt("rtty.port",5912),
					.Token = uCentral::ServiceConfig::GetString("rtty.token","nothing"),
					.TimeOut = uCentral::ServiceConfig::GetInt("rtty.timeout",60),
					.ConnectionId = CommandUUID,
					.Started = (uint64_t) time(nullptr),
					.CommandUUID = CommandUUID,
					.ViewPort = uCentral::ServiceConfig::GetInt("rtty.viewport",5913),
					};

				Poco::JSON::Object	ReturnedObject;
				Rtty.to_json(ReturnedObject);

				//	let's create the command for this request
				uCentral::Objects::CommandDetails	Cmd;
				Cmd.SerialNumber = SerialNumber;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.UUID = CommandUUID;
				Cmd.Command = uCentral::uCentralProtocol::RTTY;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::METHOD,uCentral::uCentralProtocol::RTTY);
				Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
				Params.set(uCentral::uCentralProtocol::ID, Rtty.ConnectionId);
				Params.set(uCentral::uCentralProtocol::TOKEN,Rtty.Token);
				Params.set(uCentral::uCentralProtocol::SERVER, Rtty.Server);
				Params.set(uCentral::uCentralProtocol::PORT, Rtty.Port);
				Params.set(uCentral::uCentralProtocol::USER, UserInfo_.username_);
				Params.set(uCentral::uCentralProtocol::TIMEOUT, Rtty.TimeOut);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					if(WaitForRPC(Cmd, Request, Response, 10000, false))
						ReturnObject(Request, ReturnedObject, Response);
					else
						ReturnStatus(Request, Response,Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				} else {
					ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			} else {
				NotFound(Request, Response);
				return;
			}
		} else {
			ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}
