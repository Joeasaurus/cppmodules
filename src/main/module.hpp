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

	static map<string, string> Channels = {
		{ "command", "COMMAND" },
		{ "in",      "INPUT"   },
		{ "out",     "OUTPUT"  }
	};

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

			/* tick() is the main loop of the module.
			 * To receive socket messages, run() should call pollAndProcess().
			 * The module thread will close when run() returns.
			 */
			virtual void tick(){};
			virtual void setup()=0;
			inline bool pollAndProcess();
			/* process_message() is how you handle incoming messages.
			 * Any messages that are found waiting by pollAndProcess() will come through here.
			 */
			virtual bool process_message(const Message& wMsg)=0;
			/* name() returns __info.name
			 */
			inline string name() const;

			inline void setSocketContext(shared_ptr<context_t> context);
			inline bool areSocketsOpen() const;
			inline bool areSocketsValid() const;
			
		protected:
			inline void openSockets(string parent = "__bind__");
			inline void closeSockets();

			inline string getChannel(string in);

			inline void subscribe(string channel);

			inline bool sendMessage(Message wMsg);

			template<typename retType>
			inline retType recvMessage(function<retType(const Message&)> callback, long timeout=1000);
			template<typename retType>
			inline retType recvMessage(socket_t* socket, function<retType(const Message&)> callback);

			inline vector<string> tokeniseString(const string& message, const string& spchar = " ");

			inline void errLog(string message) const;
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
		return (this->inp_in->connected() && this->inp_out->connected());
	};


/* **********
 * Protected Functions
 */
	void Module::openSockets(string parent) {
		if (!this->areSocketsOpen()) {
			auto nm = this->name();
			string inPoint = "inproc://" + nm + ".sub";
			string outPoint = "inproc://" + nm + ".pub";

			try {
				this->inp_in = new socket_t(*this->inp_context, socket_type::sub);
				this->inp_out = new socket_t(*this->inp_context, socket_type::pub);

				if (parent == "__bind__") {
					this->inp_in->bind(inPoint.c_str());
					this->inp_out->bind(outPoint.c_str());
				} else {
					this->inp_in->connect(inPoint.c_str());
					this->inp_out->connect(outPoint.c_str());

					this->subscribe(nm + "-" + Channels["command"]);
					this->subscribe(nm + "-" + Channels["in"]);
				}

				this->subscribe(Channels["command"]);
				this->subscribe(Channels["in"]);

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
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		}

		delete this->inp_in;
		delete this->inp_out;
	};
	
	void Module::subscribe(string channel) {
		try {
			this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
		
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		}
	};


	bool Module::sendMessage(Message wMsg) {
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
			sendOk = this->inp_out->send(zmqObject);
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		
		}

		return sendOk;
	};

	string Module::getChannel(string in) {
		auto t = tokeniseString(in, "-");
		cout << t.at(0) << endl;
		return t.at(0);
	};

	//TODO: Could we template the SocketType too?
	template<typename retType>
	retType Module::recvMessage(function<retType(const Message&)> callback, long timeout) {
		Message wMsg;

		pollitem_t pollSocketItems[] = {
			{ (void*)*this->inp_in, 0, ZMQ_POLLIN, 0 }
		};

		message_t zMessage;

		try {
			if (zmq::poll(pollSocketItems, 1, timeout) > 0 && inp_in->recv(&zMessage)) {
				// tokeniseString needs replacing with something that will only grab the module name
				vector<string> jsonMsg = tokeniseString(string(static_cast<char*>(zMessage.data()), zMessage.size()));
				
				wMsg.CHANNEL  = getChannel(jsonMsg.at(0));

				jsonMsg.erase(jsonMsg.begin());
				auto messageText = algorithm::join(jsonMsg, " ");

				wMsg.fromString(messageText);

			}

		} catch (const zmq::error_t &e) {
			errLog(e.what());

		}

		return callback(wMsg);
	};
			

	bool Module::pollAndProcess() {
		// Here we listen on the socket for close messages
		// false will close us so we return that for a close message
		// If the message is not a close but is for us,
		//   we return the module's process_message
		// Else just return true to keep running but not care for the message
		return this->recvMessage<bool>([&](const Message& wMsg) {
			// We trust 'Spine' for commands implicitly....
			if (wMsg.CHANNEL == Channels["command"]) {
				auto command = wMsg["data"]["command"].asString();
				if (command == "close") {
					_logger.log(name(), "Heard close command...", true);
					return false;
				} else if (command == "config-update") {
					if (wMsg["data"].isMember("config")) {
						this->config.fromJson(wMsg["data"]["config"]);
						//_logger.log(name(), this->config.asString(), true);
					}
					return true;
				}
				return this->process_message(wMsg);
			}
			return true;
		});
		_logger.log(name(), "POLLED", true);

	};

	vector<string> Module::tokeniseString(const string& message, const string& spchar) {
		vector<string> messageTokens;
		if (!message.empty()) {
			boost::split(messageTokens, message, boost::is_any_of(spchar));
			}

		return messageTokens;
	};


/* **********
 * Private Functions
 */
 	void Module::errLog(string message) const {
		_logger.getLogger()->error(this->name() + ": " + message);
	};
}

/* Export the module
 * 
 * These functions should be overriden and set 'export "C"' on.
 * These functions allow us to load the module dynamically via <dlfcn.h>
 */
typedef cppm::Module* Module_ctor(void);
typedef void Module_dctor(cppm::Module*);