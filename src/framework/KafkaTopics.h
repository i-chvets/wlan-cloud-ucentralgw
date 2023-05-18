//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
namespace OpenWifi::KafkaTopics {
	static const char * HEALTHCHECK = "healthcheck";
	static const char * STATE = "state";
	static const char * CONNECTION = "connection";
	static const char * WIFISCAN = "wifiscan";
	static const char * ALERTS = "alerts";
	static const char * COMMAND = "command";
	static const char * SERVICE_EVENTS = "service_events";
	static const char * DEVICE_EVENT_QUEUE = "device_event_queue";
	static const char * DEVICE_TELEMETRY = "device_telemetry";
	static const char * PROVISIONING_CHANGE = "provisioning_change";

	namespace ServiceEvents {
		static const char * EVENT_JOIN = "join";
		static const char * EVENT_LEAVE = "leave";
		static const char * EVENT_KEEP_ALIVE = "keep-alive";
		static const char * EVENT_REMOVE_TOKEN = "remove-token";

		namespace Fields {
			static const char * EVENT = "event";
			static const char * ID = "id";
			static const char * TYPE = "type";
			static const char * PUBLIC = "publicEndPoint";
			static const char * PRIVATE = "privateEndPoint";
			static const char * KEY = "key";
			static const char * VRSN = "version";
			static const char * TOKEN = "token";
		} // namespace Fields
	}	  // namespace ServiceEvents
} // namespace OpenWifi::KafkaTopics
