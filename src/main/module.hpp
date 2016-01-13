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

#include <boost/algorithm/string.hpp>
#include "zmq.hpp"
// Lets remove this assert while we mess with travis-ci and it's outta date packages
//static_assert(ZMQ_VERSION == 40102, "ZMQ Version 40102 is required!");

#include "main/logger.hpp"
#include "main/message.hpp"

using namespace std;
using namespace zmq;
namespace algorithm = boost::algorithm;

namespace cppm {
	class ModuleCOM;
	typedef struct ModuleInfo {
		string name = "undefined module";
		string author = "mainline";
	} ModuleInfo;

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
		friend class ModuleCOM;
		// Variables first
		protected:
			ModuleInfo __info;
			shared_ptr<context_t> inp_context;
			Logger _logger;
			chrono::system_clock::time_point timeNow;
			Message config;
		private:
			socket_t* inp_manage_in;
			socket_t* inp_manage_out;
			socket_t* inp_in;
			socket_t* inp_out;
			chrono::milliseconds timeDelta;
			bool socketsOpen = false;

		// Now our functions
		public:
			Module(string name, string author) {
				this->__info.name   = name;
				this->__info.author = author;
				this->timeNow = chrono::system_clock::now(); 
			};
			virtual ~Module(){};

			/* run() is the main loop of the module.
			 * To receive socket messages, run() should call pollAndProcess().
			 * The module thread will close when run() returns.
			 */
			virtual void run()=0;
			/* process_message() is how you handle incoming messages.
			 * Any messages that are found waiting by pollAndProcess() will come through here.
			 */
			virtual bool process_message(const Message& wMsg, CatchState cought, SocketType sockT)=0;
			/* name() returns __info.name
			 */
			inline string name() const;

			inline void setSocketContext(shared_ptr<context_t> context);
			inline bool areSocketsOpen() const;
			inline bool areSocketsValid() const;
			
		protected:
			inline void openSockets();
			inline void closeSockets();

			inline void notify(SocketType sockT, string endpoint);
			inline void subscribe(string channel);

			inline bool sendMessage(SocketType sockT, Message wMsg);
			inline bool sendMessageRecv(SocketType sockT, Message wMsg, function<bool(const  Message&)> callback);

			template<typename retType>
			inline retType recvMessage(SocketType sockT, function<retType(const Message&)> callback, long timeout=1000);
			template<typename retType>
			inline retType recvMessage(socket_t* socket, function<retType(const Message&)> callback);

			inline bool pollAndProcess();
			inline vector<string> tokeniseString(const string& message);

			inline void errLog(string message) const;

		private:
			inline bool catchAndProcess(socket_t* socket, SocketType sockT);
	};

/* **********
 * Public Functions
 */
	string Module::name() const {
		return this->__info.name;
	};


	void Module::setSocketContext(shared_ptr<context_t> context)
	{
		this->inp_context = context;
	};
	
	bool Module::areSocketsOpen() const {
		return this->socketsOpen;
	};

	bool Module::areSocketsValid() const {
		return (this->inp_in->connected()        &&
				this->inp_out->connected()       &&
				this->inp_manage_in->connected() &&
				this->inp_manage_out->connected()
		);
	};


/* **********
 * Protected Functions
 */
	void Module::openSockets() {
		if (!this->areSocketsOpen()) {
			string inPoint = "inproc://" + this->name() + ".in";
			string managePoint = "inproc://" + this->name() + ".manage";

			try {
				this->inp_in = new socket_t(*this->inp_context, socket_type::sub);
				this->inp_out = new socket_t(*this->inp_context, socket_type::pub);
				this->inp_manage_in = new socket_t(*this->inp_context, socket_type::rep);
				this->inp_manage_out = new socket_t(*this->inp_context, socket_type::req);

				this->inp_in->bind(inPoint.c_str());
				this->inp_manage_in->bind(managePoint.c_str());

				_logger.log(this->name(), "Sockets Open!", true);
				this->socketsOpen = true;

			} catch (const zmq::error_t &e) {
				errLog(e.what());
			}
		}
	};

	void Module::closeSockets() {
		try {
			this->inp_in->close();
			this->inp_out->close();
			this->inp_manage_in->close();
			this->inp_manage_out->close();
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		}

		delete this->inp_in;
		delete this->inp_out;
		delete this->inp_manage_in;
		delete this->inp_manage_out;
	};


	void Module::notify(SocketType sockT, string endpoint) {
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
			errLog(e.what());
		}
	};
	
	void Module::subscribe(string channel) {
		try {
			this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
		
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		}
	};


	bool Module::sendMessage(SocketType sockT, Message wMsg) {
		bool sendOk = false;

		string message_string = cppm::asString(wMsg["destination"]);
		//TODO: This is well ugly, maybe we'll change JSON libraries again later.
		//      We just need to be able to say "Please print this without the quotes"!
		// We get "Modules" -> Modules
		message_string.erase(0, 1);
		message_string.erase(message_string.length()-1, 1);
		message_string += " " + wMsg.asString();

		message_t zmqObject(message_string.length());
		memcpy(zmqObject.data(), message_string.data(), message_string.length());

		try {

			if (sockT == SocketType::PUB || sockT == SocketType::SUB) {
				sendOk = this->inp_out->send(zmqObject);

			} else if (sockT == SocketType::MGM_IN) {
				sendOk = this->inp_manage_in->send(zmqObject);

			} else if (sockT == SocketType::MGM_OUT) {
				_logger.getLogger()->warn("{}: {}", this->name(), "Module::sendMessage with SocketType::MGM_OUT called, but was missing a callback!");
				sendOk = this->inp_manage_out->send(zmqObject);

			}
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		
		}

		return sendOk;
	};

	bool Module::sendMessageRecv(SocketType sockT, Message wMsg, function<bool(const  Message&)> callback) {
		bool sendOk = false;

		if (sockT == SocketType::PUB || sockT == SocketType::SUB ||	sockT == SocketType::MGM_IN ) {
			_logger.getLogger()->warn("{}: {}", this->name(), "Module::sendMessageRecv called with extraneous callback. Only SocketType::MGM_OUT is supported.");
			sendOk = this->sendMessage(sockT, wMsg);

		} else if (sockT == SocketType::MGM_OUT) {
			if (this->sendMessage(sockT, wMsg)) {
				sendOk = this->recvMessage<bool>(SocketType::MGM_OUT, [&](const Message& message) {
					return callback(message);
				}, 5000);

			} else {
				sendOk = false;

			}
		}

		return sendOk;
	};


	//TODO: Could we template the SocketType too?
	template<typename retType>
	retType Module::recvMessage(SocketType sockT, function<retType(const Message&)> callback, long timeout) {
		Message wMsg;

		socket_t* pollSocket;
		pollitem_t pollSocketItems[1];
		pollSocketItems[0].events = ZMQ_POLLIN;

		message_t zMessage;

		//TODO: Make this a separate function
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
			_logger.log(name(), "Error! SocketType not supported!", true);
			return false;

		}

		try {
			if (zmq::poll(pollSocketItems, 1, timeout) > 0) {
				if(pollSocket->recv(&zMessage)) {
					// tokeniseString needs replacing with something that will only grab the module name
					vector<string> jsonMsg = tokeniseString(string(static_cast<char*>(zMessage.data()), zMessage.size()));
					
					jsonMsg.erase(jsonMsg.begin());
					auto messageText = algorithm::join(jsonMsg, " ");

					wMsg.fromString(messageText);

				}

			}

		} catch (const zmq::error_t &e) {
			errLog(e.what());

		}

		return callback(wMsg);
	};

	template<typename retType>
	retType Module::recvMessage(socket_t* socket, function<retType(const Message&)> callback) {
		Message wMsg;
		message_t zMessage;

		if(socket->recv(&zMessage)) {
			// tokeniseString needs replacing with something that will only grab the module name
			vector<string> jsonMsg = tokeniseString(string(static_cast<char*>(zMessage.data()), zMessage.size()));
			
			jsonMsg.erase(jsonMsg.begin());
			auto messageText = algorithm::join(jsonMsg, " ");

			wMsg.fromString(messageText);

		}

		return callback(wMsg);
	};
			

	bool Module::pollAndProcess() {
		int pollSocketCount = 2;
		pollitem_t pollSocketItems[] = {
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

	vector<string> Module::tokeniseString(const string& message) {
		vector<string> messageTokens;
		if (!message.empty()) {
			boost::split(messageTokens, message, boost::is_any_of(" "));
			}

		return messageTokens;
	};


/* **********
 * Private Functions
 */
 	void Module::errLog(string message) const {
		_logger.getLogger()->error(this->name() + ": " + message);
	};


	bool Module::catchAndProcess(socket_t* socket, SocketType sockT){
		// Here we listen on the socket we're told to for close messages
		// false will close us so we return that for a close message
		// If the message is not a close but is for us,
		//   we return the module's process_message
		// Else just return true to keep running but not care for the message
		return this->recvMessage<bool>(socket, [&](const Message& wMsg) {
			CatchState cought = CatchState::NOT_FOR_ME;

			// Only if it's for us or 'Everyone'
			if (wMsg["destination"].asString() == "Modules") {
				cought = CatchState::FOR_ALL;
			}
			else if (wMsg["destination"].asString() == this->name()) {
				cought = CatchState::FOR_ME;
			}
			else {
				_logger.log(name(), "NOT FOR ME", true);
				return true;
			}

			// We trust 'Spine' for commands implicitly....
			if (wMsg["source"].asString() == "Spine" && wMsg["data"].isMember("command")) {
				auto command = wMsg["data"]["command"].asString();
				if (command == "close") {
					_logger.log(name(), "Heard close command...", true);
					return false;
				} else if (command == "config-update") {
					if (wMsg["data"].isMember("config")) {
						this->config.fromJson(wMsg["data"]["config"]);
						_logger.log(name(), this->config.asString(), true);
					}
				}
			}
			return this->process_message(wMsg, cought, sockT);
		});
	};
}

/* Export the module
 * 
 * These functions should be overriden and set 'export "C"' on.
 * These functions allow us to load the module dynamically via <dlfcn.h>
 */
typedef cppm::Module* Module_ctor(void);
typedef void Module_dctor(cppm::Module*);