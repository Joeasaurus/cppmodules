#pragma once
// Common
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>

#include <thread>
#include <chrono>
#include <functional>

#include <zmq.hpp>
#include <boost/algorithm/string.hpp>

#include "lib/spdlog/spdlog.h"
#include "lib/cpp-json/json.h"

using namespace std;
namespace algorithm = boost::algorithm;

typedef struct ModuleInfo {
	string name = "undefined module";
	string author = "mainline";
} ModuleInfo;

typedef struct M_Message {
	json::object message;
} M_Message;

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
			this->setLogger();
		};
		virtual ~Module(){};
		virtual bool run()=0;
		virtual bool process_message(const json::value& message, CatchState cought, SocketType sockT)=0;
		string name()
		{
			return this->__info.name;
		};
		void setLogger()
		{
			this->logger = spdlog::get("SpineLogger");
			this->logger->debug("{}: Logger Open", this->name());
		};
		void setSocketContext(zmq::context_t* context)
		{
			this->inp_context = context;
		};
		void openSockets()
		{
			string inPoint = "inproc://" + this->name() + ".in";
			string managePoint = "inproc://" + this->name() + ".manage";
			try {
				this->inp_in = new zmq::socket_t(*this->inp_context, ZMQ_SUB);
				this->inp_out = new zmq::socket_t(*this->inp_context, ZMQ_PUB);
				this->inp_manage_in = new zmq::socket_t(*this->inp_context, ZMQ_REP);
				this->inp_manage_out = new zmq::socket_t(*this->inp_context, ZMQ_REQ);

				this->inp_in->bind(inPoint.c_str());
				this->inp_manage_in->bind(managePoint.c_str());
				this->logger->debug("{}: Sockets Open", this->name());
			} catch (const zmq::error_t &e) {
				cout << e.what() << endl;
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
				cout << e.what() << endl;
			}
		};
		void subscribe(string channel)
		{
			try {
				this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
			} catch (const zmq::error_t &e) {
				cout << e.what() << endl;
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
		shared_ptr<spdlog::logger> logger;
		zmq::context_t* inp_context;
		chrono::system_clock::time_point timeNow;
		json::object config;
		void createEvent(string title, chrono::milliseconds interval,
			function<bool(chrono::milliseconds delta)> callback
		) {
			// auto ttp = chrono::system_clock::to_time_t(chrono::system_clock::now() + interval);
			// this->logger->debug(ctime(&ttp));
			this->events.insert(pair<string, Event>(
				title, Event{
					chrono::milliseconds(0),
					interval,
					callback
				}
			));
		};
		M_Message newMessage(string destination, json::object data)
		{
			return M_Message{
				json::object{
					{ "source", this->name() },
					{ "destination", destination },
					{ "data",  data }
				}
			};
		};
		string nameMsg(string message) {
			return this->name() + ": " + message;
		};
		void nameMsg(string& message) {
			message = this->name() + ": " + message;
		};
		bool sendMessage(SocketType sockT, string destination,
						 json::object msg)
		{
			bool sendOk = false;

			M_Message message = newMessage(destination, msg);
			string message_string = destination + " " + json::stringify(message.message);

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
		bool sendMessageRecv(SocketType sockT, string destination,
						 json::object msg, function<bool(const json::value&)> callback)
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
							function<retType(const json::value&)> callback,
							long timeout=1000
		) {
			string messageText("{}");

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
						messageText = string(static_cast<char*>(message.data()), message.size());
					}
				}
			} catch (const zmq::error_t &e) {
				cout << e.what() << endl;
			}
			// I know this is silly, we can't rely on pretty print because values are arbitray
			//  and may have spaces.
			// tokeniseString needs replacing with something that will only grab the module name
			if (messageText != "{}" && messageText != "") {
				vector<string> jsonMsg = tokeniseString(messageText);
				jsonMsg.erase(jsonMsg.begin());
				messageText = algorithm::join(jsonMsg, " ");
			}
			return callback(json::parse(messageText));
		};
		template<typename retType>
		retType recvMessage(zmq::socket_t* socket,
						function<retType(const json::value&)> callback
		) {
			string messageText("{}");
			zmq::message_t message;

			if(socket->recv(&message)) {
				messageText = string(static_cast<char*>(message.data()), message.size());
			}

			// I know this is silly, we can't rely on pretty print because values are arbitray
			//  and may have spaces.
			// tokeniseString needs replacing with something that will only grab the module name
			if (messageText != "{}" && messageText != "") {
				vector<string> jsonMsg = tokeniseString(messageText);
				jsonMsg.erase(jsonMsg.begin());
				messageText = algorithm::join(jsonMsg, " ");
			}
			return callback(json::parse(messageText));
		};
		void closeSockets()
		{
			try {
				this->inp_in->close();
				this->inp_out->close();
				this->inp_manage_in->close();
				this->inp_manage_out->close();
			} catch (const zmq::error_t &e) {
				cout << e.what() << endl;
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
				{ *this->inp_manage_in, 0, ZMQ_POLLIN, 0 },
				{ *this->inp_in, 0, ZMQ_POLLIN, 0 }
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
				[&](const json::value& message) {
					CatchState cought = CatchState::NOT_FOR_ME;
					// this->logger->debug(json::stringify(message, json::PRETTY_PRINT));
// Messages from the Spine
					if (to_string(message["destination"]) == "Modules" ||
						to_string(message["destination"]) == this->name()
					   ) {
						cought = CatchState::FOR_ME;
						if (to_string(message["source"]) == "Spine") {
							if (json::has_key(message["data"], "command")) {
								auto command = to_string(message["data"]["command"]);
								if (command == "close") {
									return false;
								} else if (command == "config-update") {
									if (json::has_key(message["data"], "config")) {
										this->config = json::as_object(message["data"]["config"]);
									}
								}
							} else if (json::has_key(message["data"], "ClockTick")) {
								return this->tickTimer(
									chrono::milliseconds(
										stoll(json::to_string(message["data"]["ClockTick"]))
									)
								);
							}
						}
					}
					return this->process_message(message, cought, sockT);
			});
		};
};

typedef Module* Module_ctor(void);
typedef void Module_dctor(Module*);