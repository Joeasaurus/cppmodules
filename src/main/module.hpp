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
#include "main/messages/command.hpp"
#include "main/messages/input.hpp"
#include "main/messages/output.hpp"

using namespace std;
using namespace zmq;
namespace algorithm = boost::algorithm;

namespace cppm {
	using namespace messages;
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
			virtual bool process_command(const Message& wMsg)=0;
			virtual bool process_input(const Message& wMsg)=0;
			virtual bool process_output(const Message& wMsg){return false;}; // not all modules care for output
			virtual bool process_message(const Message& wMsg){return false;}; // not all modules care for junk
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

			inline void subscribe(CHANNEL chan);

			inline bool sendMessage(Message wMsg);

			template<typename retType>
			inline retType recvMessage(function<retType(const Message&)> callback, long timeout=1000);
			template<typename retType>
			inline retType recvMessage(socket_t* socket, function<retType(const Message&)> callback);

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
			auto nm = name();
			string inPoint = "inproc://";
			string outPoint = "inproc://";

			try {
				inp_in = new socket_t(*inp_context, socket_type::sub);
				inp_out = new socket_t(*inp_context, socket_type::pub);

				if (parent == "__bind__") {
					inPoint += nm + ".sub";
					outPoint += nm + ".pub";
					inp_in->bind(inPoint.c_str());
					inp_out->bind(outPoint.c_str());
					subscribe(CHANNEL::Out);
				} else {
					// We sub their pub and v/v
					inPoint += parent + ".pub";
					outPoint += parent + ".sub";
					inp_in->connect(inPoint.c_str());
					inp_out->connect(outPoint.c_str());

					// Here is the only place we need a delim,
					// when the modules need to hear messages for themselves only.
				}

				subscribe(CHANNEL::In);
				subscribe(CHANNEL::Cmd);

				_logger.log(name(), "Sockets Open!", true);
				socketsOpen = true;

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
	
	void Module::subscribe(CHANNEL chan) {
		try {
			auto channel = chanToStr[chan];
			this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
		
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		}
	};


	bool Module::sendMessage(Message wMsg) {
		bool sendOk = false;

		auto message_string = wMsg.format();
		// _logger.log(name(), message_string);

		message_t zmqObject(message_string.length());
		memcpy(zmqObject.data(), message_string.data(), message_string.length());

		try {
			sendOk = this->inp_out->send(zmqObject);
		} catch (const zmq::error_t &e) {
			errLog(e.what());
		
		}

		return sendOk;
	};

	//TODO: Could we template the SocketType too?
	template<typename retType>
	retType Module::recvMessage(function<retType(const Message&)> callback, long timeout) {
		pollitem_t pollSocketItems[] = {
			{ (void*)*this->inp_in, 0, ZMQ_POLLIN, 0 }
		};

		message_t zMessage;

		try {
			if (zmq::poll(pollSocketItems, 1, timeout) > 0 && inp_in->recv(&zMessage)) {
				// tokeniseString needs replacing with something that will only grab the module name
				auto normMsg = string(static_cast<char*>(zMessage.data()), zMessage.size());
				Message msg;
				msg.payload(normMsg, false);
				_logger.log(name(), normMsg);
				return callback(msg); 
			}

		} catch (const zmq::error_t &e) {
			errLog(e.what());

		}

		Message msg;
		return callback(msg);
	};
			

	bool Module::pollAndProcess() {
		// Here we listen on the socket for close messages
		// false will close us so we return that for a close message
		// If the message is not a close but is for us,
		//   we return the module's process_message
		// Else just return true to keep running but not care for the message
		return this->recvMessage<bool>([&](const Message& msg) {
			switch (msg._chan) {
				case CHANNEL::None:
					return false;
				case CHANNEL::Cmd:
					return process_command(msg);
				case CHANNEL::In:
					return process_input(msg);
				case CHANNEL::Out:
					return process_output(msg);
			}
			
			return this->process_message(msg);
		});
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