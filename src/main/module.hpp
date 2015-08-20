#pragma once
// Common
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>

#include <thread>
#include <chrono>
#include <functional>

#include <boost/algorithm/string.hpp>
#include <zmq.hpp>
static_assert(ZMQ_VERSION == 40102, "ZMQ Version 40102 is required!");

#include "spdlog/spdlog.h"
#include "json/json.h"

using namespace std;
using namespace zmq;
namespace algorithm = boost::algorithm;

typedef struct ModuleInfo {
	string name = "undefined module";
	string author = "mainline";
} ModuleInfo;

typedef struct WireMessage {
	Json::Value message;
	Json::Reader reader;
	Json::StreamWriterBuilder wbuilder;
	WireMessage() {
		message["data"] = Json::objectValue;
		message["source"] = "";
		message["destination"] = "";
		wbuilder["indentation"] = "";
	};
	WireMessage(string name, string destination) {
		message["data"] = Json::objectValue;
		message["source"] = name;
		message["destination"] = destination;
		wbuilder["indentation"] = "";
	};
	void setMessage(Json::Value inMessage) {
		message = inMessage;
	};
	Json::Value& operator[](const string arrayKey) {
        return message[arrayKey];
    };
	const Json::Value& operator[](const string arrayKey) const {
        return message[arrayKey];
    };
    string asString(const Json::Value& printable) {
		return Json::writeString(wbuilder, printable);
    };
    string asString() const {
    	return Json::writeString(wbuilder, message);
    };
    void parseIn(const string& parsable) {
    	reader.parse(parsable, message, false);
    };
    void parseIn(Json::Value& value, const string& parsable) {
    	reader.parse(parsable, value, false);
    };
    void parseInFile(const string& parsable) {
    	parseInFile(message, parsable);
    };
    void parseInFile(Json::Value& value, const string& parsable) {
    	ifstream fileStream(parsable);
    	if (fileStream) {
			stringstream buffer;
			buffer << fileStream.rdbuf();
	    	reader.parse(buffer.str(), value, false);
    	} else {
    		//TODO: Improve this!
    		throw 50;
    	}
    };
} WireMessage;

typedef struct Event {
	chrono::milliseconds delta;
	chrono::milliseconds interval;
	function<bool(chrono::milliseconds delta)> callback;
} Event;

enum class MgmtCommand {
	REGISTER,
	CLOSE
};

enum class CatchState {
	NO_TOKENS,
	CLOSE_HEARD,
	NOT_FOR_ME,
	FOR_ALL,
	FOR_ME
};

enum class SocketType {
	PUB,
	SUB,
	MGM_IN,
	MGM_OUT
};

class Module {
	public:
		Module(string name, string author)
		{
			this->__info.name   = name;
			this->__info.author = author;
			this->timeNow = chrono::system_clock::now();
		};
		virtual ~Module(){};
		virtual bool run()=0;
		virtual bool process_message(const WireMessage& wMsg, CatchState cought, SocketType sockT)=0;
		string name()
		{
			return this->__info.name;
		};
		void setLogger(const string& loggerName)
		{
			this->logger = spdlog::get(loggerName);
			this->logger->debug("{}: Logger Open", this->name());
		};
		shared_ptr<spdlog::logger> getLogger() const {
			return this->logger;
		};
		void setSocketContext(zmq::context_t* context)
		{
			this->inp_context = context;
		};
		bool areSocketsOpen() {
			return this->socketsOpen;
		};
		bool areSocketsValid() {
			return (this->inp_in->connected() && this->inp_out->connected() && this->inp_manage_in->connected() &&
					this->inp_manage_out->connected());
		};
		void openSockets()
		{
			if (!this->areSocketsOpen()) {
				string inPoint = "inproc://" + this->name() + ".in";
				string managePoint = "inproc://" + this->name() + ".manage";
				try {
					this->inp_in = new zmq::socket_t(*this->inp_context, ZMQ_SUB); //zmq::socket_type::sub);
					this->inp_out = new zmq::socket_t(*this->inp_context, ZMQ_PUB); //zmq::socket_type::pub);
					this->inp_manage_in = new zmq::socket_t(*this->inp_context, ZMQ_REP); //zmq::socket_type::rep);
					this->inp_manage_out = new zmq::socket_t(*this->inp_context, ZMQ_REQ); //zmq::socket_type::req);

					this->inp_in->bind(inPoint.c_str());
					this->inp_manage_in->bind(managePoint.c_str());
					this->logger->debug("{}: Sockets Open", this->name());
					this->socketsOpen = true;
				} catch (const zmq::error_t &e) {
					this->logger->error(this->nameMsg(e.what()));
				}
			}
		};
		void notify(SocketType sockT, string endpoint)
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
				this->logger->error(this->nameMsg(e.what()));
			}
		};
		void subscribe(string channel)
		{
			try {
				this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
			} catch (const zmq::error_t &e) {
				this->logger->error(this->nameMsg(e.what()));
			}
		};
		bool tickTimer(
			chrono::milliseconds newDelta
		) {
			this->timeNow += newDelta;
			this->timeDelta = newDelta;
			return this->checkEventTimer(newDelta);
		};

// PROTECTED
	protected:
		ModuleInfo __info;
		zmq::context_t* inp_context;
		shared_ptr<spdlog::logger> logger;
		chrono::system_clock::time_point timeNow;
		WireMessage config;
		void createEvent(string title, chrono::milliseconds interval,
						 function<bool(chrono::milliseconds delta)> callback
		) {
			// auto ttp = chrono::system_clock::to_time_t(chrono::system_clock::now() + interval);
			// this->logger->debug(ctime(&ttp));
			try{
				this->events.insert(pair<string, Event>(
					title, Event{
						chrono::milliseconds(0),
						interval,
						callback
					}
				));
				this->logger->debug(this->nameMsg(title + " event created"));
			} catch (const exception& e) {
				this->logger->warn(this->nameMsg(e.what()));
			}
		};
		string nameMsg(string message) {
			return this->name() + ": " + message;
		};
		void nameMsg(string& message) {
			message = this->name() + ": " + message;
		};
		bool sendMessage(SocketType sockT, WireMessage wMsg)
		{
			bool sendOk = false;

			string message_string = wMsg.asString(wMsg["destination"]);
			//TODO: This is well ugly, maybe we'll change JSON libraries again later.
			//      We just need to be able to say "Please print this without the quotes"!
			// We get "Modules" -> Modules
			message_string.erase(0, 1);
			message_string.erase(message_string.length()-1, 1);
			message_string += " " + wMsg.asString();

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
				logger->debug(this->nameMsg(e.what()));
			}

			return sendOk;
		};
		bool sendMessageRecv(SocketType sockT, WireMessage wMsg, function<bool(const  WireMessage&)> callback)
		{
			bool sendOk = false;
			if (sockT == SocketType::PUB || sockT == SocketType::SUB ||
				sockT == SocketType::MGM_IN
			) {
				this->logger->warn("{}: {}", this->name(),
					"Module::sendMessageRecv called with extraneous callback. Only SocketType::MGM_OUT is supported.");
				sendOk = this->sendMessage(sockT, wMsg);
			} else if (sockT == SocketType::MGM_OUT)
			{
				if (this->sendMessage(sockT, wMsg)) {
					sendOk = this->recvMessage<bool>(SocketType::MGM_OUT,
						[&](const WireMessage& message)
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
							function<retType(const WireMessage&)> callback,
							long timeout=1000
		) {
			WireMessage wMsg;

			zmq::socket_t* pollSocket;
			zmq::pollitem_t pollSocketItems[1];
			pollSocketItems[0].events = ZMQ_POLLIN;

			zmq::message_t zMessage;

			if (sockT == SocketType::SUB || sockT == SocketType::PUB) {
				pollSocketItems[0].socket = (void*)*this->inp_in;
				pollSocket = this->inp_in;
			} else if (sockT == SocketType::MGM_IN) {
				pollSocketItems[0].socket = (void*)*this->inp_manage_in;
				pollSocket = this->inp_manage_in;
			} else if (sockT == SocketType::MGM_OUT) {
				pollSocketItems[0].socket = (void*)*this->inp_manage_out;
				pollSocket = this->inp_manage_out;
			} else {
				this->logger->debug(this->nameMsg("Error! SocketType not supported!"));
				return false;
			}

			try {
				if (zmq::poll(pollSocketItems, 1, timeout) > 0) {
					if(pollSocket->recv(&zMessage)) {
						// tokeniseString needs replacing with something that will only grab the module name
						vector<string> jsonMsg = tokeniseString(string(static_cast<char*>(zMessage.data()), zMessage.size()));
						jsonMsg.erase(jsonMsg.begin());
						auto messageText = algorithm::join(jsonMsg, " ");
						wMsg.parseIn(messageText);
					}
				}
			} catch (const zmq::error_t &e) {
				this->logger->error(this->nameMsg(e.what()));
			}
			return callback(wMsg);
		};
		template<typename retType>
		retType recvMessage(zmq::socket_t* socket,
						function<retType(const WireMessage&)> callback
		) {
			WireMessage wMsg;
			zmq::message_t zMessage;

			if(socket->recv(&zMessage)) {
				// I know this is silly, we can't rely on pretty print because values are arbitray
				//  and may have spaces.
				// tokeniseString needs replacing with something that will only grab the module name
				vector<string> jsonMsg = tokeniseString(string(static_cast<char*>(zMessage.data()), zMessage.size()));
				jsonMsg.erase(jsonMsg.begin());
				auto messageText = algorithm::join(jsonMsg, " ");
				wMsg.parseIn(messageText);
			}

			return callback(wMsg);
		};
		void closeSockets()
		{
			try {
				this->inp_in->close();
				this->inp_out->close();
				this->inp_manage_in->close();
				this->inp_manage_out->close();
			} catch (const zmq::error_t &e) {
				this->logger->error(this->nameMsg(e.what()));
			}

			delete this->inp_in;
			delete this->inp_out;
			delete this->inp_manage_in;
			delete this->inp_manage_out;
		};
		bool pollAndProcess()
		{
			int pollSocketCount = 2;
			zmq::pollitem_t pollSocketItems[] = {
				{ (void*)*this->inp_manage_in, 0, ZMQ_POLLIN, 0 },
				{ (void*)*this->inp_in, 0, ZMQ_POLLIN, 0 }
			};
			if (zmq::poll(pollSocketItems, pollSocketCount, 0) > 0) {
				if (pollSocketItems[0].revents & ZMQ_POLLIN) {
					return this->catchAndProcess(this->inp_manage_in, SocketType::MGM_IN);
				}
				if (pollSocketItems[1].revents & ZMQ_POLLIN) {
					return this->catchAndProcess(this->inp_in, SocketType::SUB);
				}
			}
			this_thread::sleep_for(chrono::milliseconds(10));
			return true;
		};
		vector<string> tokeniseString(const string& message)
		{
			vector<string> messageTokens;
			if (!message.empty()) {
				boost::split(messageTokens, message, boost::is_any_of(" "));
 			}

			return messageTokens;
		};

// PRIVATE
	private:
		zmq::socket_t* inp_manage_in;
		zmq::socket_t* inp_manage_out;
		zmq::socket_t* inp_in;
		zmq::socket_t* inp_out;
		chrono::milliseconds timeDelta;
		map<string, Event> events;
		bool socketsOpen = false;
		bool checkEventTimer(chrono::milliseconds newDelta) {
			for (auto& event : this->events) {
				//this->logger->debug(this->nameMsg(event.first));
				event.second.delta += newDelta;
				if (event.second.delta >= event.second.interval) {
					event.second.callback(event.second.delta);
					event.second.delta = chrono::milliseconds(0);
				}
			}
			return true;
		};
		bool catchAndProcess(zmq::socket_t* socket, SocketType sockT)
		{
			// Here we listen on the socket we're told to for close messages
			// false will close us so we return that for a close message
			// If the message is not a close but is for us,
			//   we return the module's process_message
			// Else just return true to keep running but not care for the message
			return this->recvMessage<bool>(socket,
				[&](const WireMessage& wMsg) {
					CatchState cought = CatchState::NOT_FOR_ME;

					if (wMsg.message["destination"].asString() == "Modules") {
						cought = CatchState::FOR_ALL;
					}
					else if (wMsg.message["destination"].asString() == this->name()) {
						cought = CatchState::FOR_ME;
					}
					else {
						return true;
					}

					if (wMsg.message["source"].asString() == "Spine") {
						if (wMsg.message["data"].isMember("command")) {
							auto command = wMsg.message["data"]["command"].asString();
							if (command == "close") {
								return false;
							} else if (command == "config-update") {
								if (wMsg.message["data"].isMember("config")) {
									this->config.setMessage(wMsg["data"]["config"]);
								}
							}
						} else if (wMsg.message["data"].isMember("ClockTick")) {
							return this->tickTimer(
								chrono::milliseconds(
									stoll(wMsg["data"]["ClockTick"].asString())
								)
							);
						}
					}
					return this->process_message(wMsg, cought, sockT);
			});
		};
};

typedef Module* Module_ctor(void);
typedef void Module_dctor(Module*);