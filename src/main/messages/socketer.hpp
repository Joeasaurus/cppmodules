#pragma once
#include "boost/predef.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <thread>
#include <map>

// We have to do some icky things on Windows!
#if BOOST_OS_WINDOWS
	__pragma(warning(push))
	__pragma(warning(disable:4127))
	#include "zmq.hpp"
	__pragma(warning(pop))
#else
	#include "zmq.hpp"
#endif

#include "main/interfaces/logger.hpp"
#include "main/messages/messages.hpp"
#include "main/messages/context.hpp"
#include "main/exceptions/exceptions.hpp"

using namespace std;
using namespace zmq;
using namespace cppm::exceptions::socketer;
using namespace cppm::interfaces;

namespace cppm { namespace messages {
    class Socketer {
        private:
            string name{"Socketer"};
            Logger _logger;
            const Context& inp_context;
            socket_t*  inp_in;
            socket_t*  inp_out;
            mutex _moduleSockMutex;
            bool _connected = false;

            map<string, function<bool(const Message&)>> processCallbacks;
			bool emit(string hookName, const Message& msg);

        public:
            Socketer(const Context& ctx) : inp_context(ctx) {
				// cout << "Calling socketer from id: " << std::this_thread::get_id() << endl;
			};
            ~Socketer();

            const Context& getContext();
            void openSockets(string name, string parent = "__bind__");
            void closeSockets();
            bool pollAndProcess();
            bool isConnected() const;

            void on(string hookName, function<bool(const Message&)> callback);

            void subscribe(CHANNEL chan);
            void subscribe(CHANNEL chan, const string& subChan);

            bool sendMessage(Message& message) const;

            template<typename retType>
            retType recvMessage(function<retType(const Message&)> callback, long timeout=1000);
    };
}}
