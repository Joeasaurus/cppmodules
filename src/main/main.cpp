#include "main/module.hpp"
#include "main/spine.hpp"

#include "lib/spdlog/spdlog.h"
#include "lib/cpp-json/json.h"

namespace sllevel = spdlog::level;

int main(int argc, char **argv) {
	// Configure the global logger
    spdlog::set_async_mode(1048576); //queue size must be power of 2

    // This is a quick fix for debug logging by managing argv ourselves
    // We'll move to an option parser later
    spdlog::level::level_enum logLevel = sllevel::info;
    if (argc > 1 && strcmp(argv[1], "debug") == 0) {
    	logLevel = sllevel::debug;
    }
    spdlog::set_level(logLevel);

    // Create a stdout logger, multi threaded
	shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt("Logger");
	logger->debug("{}: {}", "Main", "Logger initialised");

	// Open the spine and pass it our logger
	Spine *spine = new Spine;
	spine->setLogger(logger);

	// Now we create our ZMQ context for all our sockets
	zmq::context_t spineCtx(1);

	try {
		// Lets pass the context through and use it to open all out sockets
		// Then we subscribe the spine to the spine channel
		spine->setSocketContext(&spineCtx);
		logger->debug("{}: {}", "Main", "Spine open, ZMQ context set");
		spine->openSockets();
		spine->subscribe(spine->name());
	} catch (const zmq::error_t &e) {
		logger->info("Exception: {}", e.what());
 		return 1;
	}

	// Load all the modules we wrote
	spine->loadModules(spine->moduleFileLocation);
	bool spineReturn = spine->loadConfig("./modules/main.cfg");
	if (spineReturn) {
		logger->debug("{}: {}", "Main", "Running spine");
		spineReturn = spine->run();
	} else {
		logger->warn("{}: {}", "Main", "Config failed to load, shutting down");
	}

	delete spine;
	return spineReturn ? 0 : 1;
}