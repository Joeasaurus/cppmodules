#pragma once
// Common
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <thread>
#include <chrono>
#include <functional>

#include <zmq.hpp>
#include <boost/algorithm/string.hpp>
// Module Specific
#include "lib/spdlog/spdlog.h"

typedef struct ModuleInfo {
	std::string name = "undefined module";
	std::string author = "mainline";
} ModuleInfo;

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
		virtual bool process_message(const std::string& message, const std::vector<std::string>& tokens)=0;
		virtual std::string name() {
			return this->__info.name;
		};
		void setLogger(const std::shared_ptr<spdlog::logger> loggerHandle) {
			this->logger = loggerHandle;
			this->logger->debug("{}: Logger Open", this->name());
		};
		void setSocketContext(zmq::context_t* context) {
			this->inp_context = context;
		};
		void openSockets() {
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
		void closeSockets() {
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
		void notify(SocketType sockT, std::string endpoint) {
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
		void subscribe(std::string channel) {
			try{
				this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		bool sendMessage(SocketType sockT, std::string message) {
			bool sendErr = false;

			zmq::message_t zmqObject(message.size());
			memcpy(zmqObject.data(), message.data(), message.size());

			try {
				if (sockT == SocketType::PUB || sockT == SocketType::SUB) {
					sendErr = this->inp_out->send(zmqObject);
				} else if (sockT == SocketType::MGM_IN) {
					sendErr = this->inp_manage_in->send(zmqObject);
				} else if (sockT == SocketType::MGM_OUT) {
					sendErr = this->inp_manage_out->send(zmqObject);
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}

			return sendErr;
		};
		std::string recvMessage(SocketType sockT, long timeout=1000) {
			return this->recvMessage<std::string>(sockT,
				[](const std::string& message) {
					return message;
				}, timeout
			);
		};
		template<typename retType>
		retType recvMessage(SocketType sockT,
							std::function<retType(const std::string&)> callback,
							long timeout=1000
		) {
			std::string messageText;

			zmq::socket_t* pollSocket;
			zmq::pollitem_t pollSocketItems[1];
			pollSocketItems[0].events = ZMQ_POLLIN;

			zmq::message_t message;
			bool recvErr = false;

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

			int data = zmq::poll(pollSocketItems, 1, timeout);
			if (data > 0) {
				recvErr = pollSocket->recv(&message);
			}

			if(recvErr) {
				messageText = std::string(static_cast<char*>(message.data()), message.size());
			}

			return callback(messageText);
		};
		template<typename retType>
		retType recvMessage(zmq::socket_t* socket,
						std::function<retType(const std::string&)> callback,
						long timeout=1000
		) {
			std::string messageText;
			zmq::message_t message;

			if(socket->recv(&message)) {
				messageText = std::string(static_cast<char*>(message.data()), message.size());
			}

			return callback(messageText);
		};
		bool catchCloseAndProcess(zmq::socket_t* socket) {
			// Here we listen on the socket we're told to for close messages
			// false will close us so we return that for a close message
			// If the message is not a close but is for us,
			//   we return the module's process_message
			// Else just return true to keep running but not care for the message
			return this->recvMessage<bool>(socket,
				[&](const std::string& message) {
					auto tokens = tokeniseString(message);
					if (tokens.size() > 0) {
						auto modName = tokens.at(0);
						if (modName == "Module" || modName == this->name()) {
							if (tokens.at(1) == "close") {
								this->logger->debug("Close heard");
								return false;
							} else {
								return this->process_message(message, tokens);
							}
						}
					}
					return true;
				}, 0);
		};
		bool pollAndProcess(long timeout=1000) {
			int pollSocketCount = 2;
			zmq::pollitem_t pollSocketItems[] = {
				{ *this->inp_manage_in, 0, ZMQ_POLLIN, 0 },
				{ *this->inp_in, 0, ZMQ_POLLIN, 0 }
			};

			if (zmq::poll(pollSocketItems, pollSocketCount, timeout) > 0) {
				if (pollSocketItems[0].revents & ZMQ_POLLIN) {
					return this->catchCloseAndProcess(this->inp_manage_in);
				}
				if (pollSocketItems[1].revents & ZMQ_POLLIN) {
					return this->catchCloseAndProcess(this->inp_in);
				}
			}
			return true;
		};
		std::vector<std::string> tokeniseString(const std::string& message) {
			std::vector<std::string> messageTokens;
			if (!message.empty()) {
				boost::split(messageTokens, message, boost::is_any_of(" "));
 			}

			return messageTokens;
		};

};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);