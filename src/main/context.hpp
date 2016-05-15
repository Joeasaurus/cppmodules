#pragma once

#include <memory>
#include <mutex>

// We have to do some icky things on Windows!
#if BOOST_OS_WINDOWS
	__pragma(warning(push))
	__pragma(warning(disable:4127))
	#include "zmq.hpp"
	__pragma(warning(pop))
#else
	#include "zmq.hpp"
#endif

using namespace zmq;

namespace cppm {
    class Context {
        public:
            static Context& getSingleContext() {
                static Context* ctx = new Context;
                return *ctx;
            };

            context_t* getContext() {
                return context.get();
            };

        private:
            Context() {
                if (context == nullptr)
                    context = make_shared<zmq::context_t>(1);
            };
            // delete copy and move constructors and assign operators
            // Context(Context const&);             // Copy construct
            // Context(Context&&);                  // Move construct
            // Context& operator=(Context const&);  // Copy assign
            // Context& operator=(Context &&);      // Move assign

            shared_ptr<context_t> context = nullptr;
    };

}
