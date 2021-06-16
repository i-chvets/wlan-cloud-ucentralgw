//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_COMMANDCHANNEL_H
#define UCENTRALGW_COMMANDCHANNEL_H

#include "SubSystemServer.h"

#include "Poco/File.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"

namespace uCentral {

	class CommandChannel : public SubSystemServer {
	  public:
		static CommandChannel *instance() {
			if (instance_ == nullptr) {
				instance_ = new CommandChannel;
			}
			return instance_;
		}

		int Start() override;
		void Stop() override;
		std::string  ProcessCommand(const std::string &Command);

	  private:
		static CommandChannel * instance_;
		std::unique_ptr<Poco::File> 				SocketFile_;
		std::unique_ptr<Poco::Net::SocketAddress>	UnixSocket_;
		std::unique_ptr<Poco::Net::ServerSocket>	Svs_;
		std::unique_ptr<Poco::Net::TCPServer>		Srv_;

		CommandChannel() noexcept;
	};

	inline CommandChannel * CommandChannel() { return CommandChannel::instance(); }
}		//namespace

#endif // UCENTRALGW_COMMANDCHANNEL_H