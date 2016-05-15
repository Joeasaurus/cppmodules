#pragma once
#include "boost/predef.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
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

#include "main/logger.hpp"
#include "main/messages/messages.hpp"
#include "main/exceptions/exceptions.hpp"

using namespace std;
using namespace zmq;
using namespace cppm::exceptions::socketer;

namespace cppm { namespace messages {
    class Socketer {
        private:
            string name{"Socketer"};
            Logger _logger;
            shared_ptr<context_t> inp_context;
            socket_t* inp_in;
            socket_t* inp_out;
            mutex _moduleSockMutex;
            atomic<bool> _connected{false};

            map<string, function<bool(const Message&)>> processCallbacks;
			bool emit(string hookName, const Message& msg);

        public:
            Socketer(shared_ptr<context_t> ctx);
            ~Socketer();

            shared_ptr<context_t> getContext();
            void openSockets(string name, string parent = "__bind__");
            void closeSockets();
            bool pollAndProcess();
            bool isConnected() const;

            void on(string hookName, function<bool(const Message&)> callback);

            void subscribe(CHANNEL chan);
            void subscribe(const string& chan);

            bool sendMessage(Message wMsg) const;

            template<typename retType>
            retType recvMessage(function<retType(const Message&)> callback, long timeout=1000);
    };
}}
