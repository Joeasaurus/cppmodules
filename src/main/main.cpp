#include "main/module.hpp"
#include "main/spine.hpp"

#include "lib/spdlog/spdlog.h"

int main(int argc, char **argv) {
	// Configure the global logger
    spdlog::set_async_mode(1048576); //queue size must be power of 2
    spdlog::set_level(spdlog::level::info);

    // Create a stdout logger, multi threaded
	std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt("Logger");
	logger->debug("{}: {}", "Main", "Logger initialised");

	// Open the spine and pass it out logger
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
	spine->loadModules("./modules");
	logger->info("{}: {}", "Main", "Running spine");
	bool spineReturn = spine->run();

	delete spine;
	return spineReturn ? 0 : 1;
}