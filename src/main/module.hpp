#pragma once
// Common
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <regex>

#include <thread>
#include <chrono>
#include <functional>

#include <zmq.hpp>
#include <boost/algorithm/string.hpp>

#include "lib/spdlog/spdlog.h"
#include "lib/cpp-json/json.h"

typedef struct ModuleInfo {
	std::string name = "undefined module";
	std::string author = "mainline";
} ModuleInfo;

typedef struct M_Message {
	json::object message;
} M_Message;

enum class MgmtCommand {
	REGISTER,
	CLOSE
};

enum class CatchState {
	NO_TOKENS,
	CLOSE_HEARD,
	NOT_FOR_ME,
	FOR_ME
};

enum class SocketType {
	PUB,
	SUB,
	MGM_IN,
	MGM_OUT
};

class Module {
	protected:
		ModuleInfo __info;
		zmq::context_t* inp_context;
		zmq::socket_t* inp_manage_in;
		zmq::socket_t* inp_manage_out;
		zmq::socket_t* inp_in;
		zmq::socket_t* inp_out;
		std::shared_ptr<spdlog::logger> logger;
	public:
		virtual ~Module(){};
		virtual bool run()=0;
		virtual bool process_message(const json::value& message, CatchState cought, SocketType sockT)=0;
		virtual std::string name()
		{
			return this->__info.name;
		};
		void setLogger(const std::shared_ptr<spdlog::logger> loggerHandle)
		{
			this->logger = loggerHandle;
			this->logger->debug("{}: Logger Open", this->name());
		};
		void setSocketContext(zmq::context_t* context)
		{
			this->inp_context = context;
		};
		void openSockets()
		{
			std::string inPoint = "inproc://" + this->name() + ".in";
			std::string managePoint = "inproc://" + this->name() + ".manage";
			try {
				this->inp_in = new zmq::socket_t(*this->inp_context, ZMQ_SUB);
				this->inp_out = new zmq::socket_t(*this->inp_context, ZMQ_PUB);
				this->inp_manage_in = new zmq::socket_t(*this->inp_context, ZMQ_REP);
				this->inp_manage_out = new zmq::socket_t(*this->inp_context, ZMQ_REQ);

				this->inp_in->bind(inPoint.c_str());
				this->inp_manage_in->bind(managePoint.c_str());
				this->logger->debug("{}: Sockets Open", this->name());
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		void closeSockets()
		{
			try {
				this->inp_in->close();
				this->inp_out->close();
				this->inp_manage_in->close();
				this->inp_manage_out->close();
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}

			delete this->inp_in;
			delete this->inp_out;
			delete this->inp_manage_in;
			delete this->inp_manage_out;
		};
		void notify(SocketType sockT, std::string endpoint)
		{
			endpoint = "inproc://" + endpoint;
			try {
				if (sockT == SocketType::PUB) {
					endpoint += ".in";
					this->inp_out->connect(endpoint.c_str());
				} else if (sockT == SocketType::MGM_OUT) {
					endpoint += ".manage";
					this->inp_manage_out->connect(endpoint.c_str());
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		void subscribe(std::string channel)
		{
			try {
				this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		M_Message newMessage(std::string destination, json::object data)
		{
			return M_Message{
				json::object{
					{ "source", this->name() },
					{ "destination", destination },
					{ "data",  data }
				}
			};
		};
		bool sendMessage(SocketType sockT, std::string destination,
						 json::object msg)
		{
			bool sendOk = false;

			M_Message message = newMessage(destination, msg);
			std::string message_string = destination + " " + json::stringify(message.message);

			zmq::message_t zmqObject(message_string.length());
			memcpy(zmqObject.data(), message_string.data(), message_string.length());

			try {
				if (sockT == SocketType::PUB || sockT == SocketType::SUB) {
					sendOk = this->inp_out->send(zmqObject);
				} else if (sockT == SocketType::MGM_IN) {
					sendOk = this->inp_manage_in->send(zmqObject);
				} else if (sockT == SocketType::MGM_OUT) {
					this->logger->warn("{}: {}", this->name(),
						"Module::sendMessage with SocketType::MGM_OUT called, but was missing a callback!");
					sendOk = this->inp_manage_out->send(zmqObject);
				}
			} catch (const zmq::error_t &e) {
				this->logger->error(e.what());
			}

			return sendOk;
		};
		bool sendMessageRecv(SocketType sockT, std::string destination,
						 json::object msg, std::function<bool(const json::value&)> callback)
		{
			bool sendOk = false;
			if (sockT == SocketType::PUB || sockT == SocketType::SUB ||
				sockT == SocketType::MGM_IN
			) {
				this->logger->warn("{}: {}", this->name(),
					"Module::sendMessageRecv called with extraneous callback. Only SocketType::MGM_OUT is supported.");
				sendOk = this->sendMessage(sockT, destination, msg);
			} else if (sockT == SocketType::MGM_OUT)
			{
				if (this->sendMessage(sockT, destination, msg)) {
					sendOk = this->recvMessage<bool>(SocketType::MGM_OUT,
						[&](const json::value& message)
					{
						return callback(message);
					}, 5000);
				} else {
					sendOk = false;
				}
			}

			return sendOk;
		};
		template<typename retType>
		retType recvMessage(SocketType sockT,
							std::function<retType(const json::value&)> callback,
							long timeout=1000
		) {
			std::string messageText("{}");

			zmq::socket_t* pollSocket;
			zmq::pollitem_t pollSocketItems[1];
			pollSocketItems[0].events = ZMQ_POLLIN;

			zmq::message_t message;

			if (sockT == SocketType::SUB || sockT == SocketType::PUB) {
				pollSocketItems[0].socket = *this->inp_in;
				pollSocket = this->inp_in;
			} else if (sockT == SocketType::MGM_IN) {
				pollSocketItems[0].socket = *this->inp_manage_in;
				pollSocket = this->inp_manage_in;
			} else if (sockT == SocketType::MGM_OUT) {
				pollSocketItems[0].socket = *this->inp_manage_out;
				pollSocket = this->inp_manage_out;
			}

			try {
				int data = zmq::poll(pollSocketItems, 1, timeout);
				if (data > 0) {
					if(pollSocket->recv(&message)) {
						messageText = std::string(static_cast<char*>(message.data()), message.size());
					}
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
			// I know this is silly, we can't rely on pretty print because values are arbitray
			//  and may have spaces.
			// tokeniseString needs replacing with something that will only grab the module name
			if (messageText != "{}" && messageText != "") {
				std::vector<std::string> jsonMsg = tokeniseString(messageText);
				jsonMsg.erase(jsonMsg.begin());
				messageText = boost::algorithm::join(jsonMsg, " ");
			}
			return callback(json::parse(messageText));
		};
		template<typename retType>
		retType recvMessage(zmq::socket_t* socket,
						std::function<retType(const json::value&)> callback
		) {
			std::string messageText("{}");
			zmq::message_t message;

			if(socket->recv(&message)) {
				messageText = std::string(static_cast<char*>(message.data()), message.size());
			}

			// I know this is silly, we can't rely on pretty print because values are arbitray
			//  and may have spaces.
			// tokeniseString needs replacing with something that will only grab the module name
			if (messageText != "{}" && messageText != "") {
				std::vector<std::string> jsonMsg = tokeniseString(messageText);
				jsonMsg.erase(jsonMsg.begin());
				messageText = boost::algorithm::join(jsonMsg, " ");
			}
			return callback(json::parse(messageText));
		};
		bool catchCloseAndProcess(zmq::socket_t* socket, SocketType sockT)
		{
			// Here we listen on the socket we're told to for close messages
			// false will close us so we return that for a close message
			// If the message is not a close but is for us,
			//   we return the module's process_message
			// Else just return true to keep running but not care for the message
			return this->recvMessage<bool>(socket,
				[&](const json::value& message) {
					CatchState cought = CatchState::NOT_FOR_ME;
					// this->logger->debug(json::stringify(message, json::PRETTY_PRINT));
					if (to_string(message["source"]) == "Spine" &&
					   (to_string(message["destination"]) == "Modules" ||
						to_string(message["destination"]) == this->name()
					)) {
						cought = CatchState::FOR_ME;
						if (json::has_key(message["data"], "command")) {
							if (to_string(message["data"]["command"]) == "close") {
								return false;
							}
						}
					}
					return this->process_message(message, cought, sockT);
				});
		};
		bool pollAndProcess()
		{
			int pollSocketCount = 2;
			zmq::pollitem_t pollSocketItems[] = {
				{ *this->inp_manage_in, 0, ZMQ_POLLIN, 0 },
				{ *this->inp_in, 0, ZMQ_POLLIN, 0 }
			};

			if (zmq::poll(pollSocketItems, pollSocketCount, 0) > 0) {
				if (pollSocketItems[0].revents & ZMQ_POLLIN) {
					return this->catchCloseAndProcess(this->inp_manage_in, SocketType::MGM_IN);
				}
				if (pollSocketItems[1].revents & ZMQ_POLLIN) {
					return this->catchCloseAndProcess(this->inp_in, SocketType::SUB);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			return true;
		};
		std::vector<std::string> tokeniseString(const std::string& message)
		{
			std::vector<std::string> messageTokens;
			if (!message.empty()) {
				boost::split(messageTokens, message, boost::is_any_of(" "));
 			}

			return messageTokens;
		};

};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);