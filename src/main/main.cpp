#include "main/spine.hpp"

#include "main/segvhandler.hpp"
#include "lib/spdlog/spdlog.h"
#include "lib/cpp-json/json.h"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *    (SO boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

int main(int argc, char **argv) {
	signal(SIGSEGV, handler);
	// Configure the global logger first!!
	auto logger = Spine::createLogger(argc > 1 && strcmp(argv[1], "debug") == 0);

	// Open the spine and pass it our logger
	Spine spine;

	try {
		// Lets pass the context through and use it to open all out sockets
		spine.openSockets();
		// We used to subscribe to ourselves for introspection
		//spine.subscribe(spine.name());
	} catch (const zmq::error_t &e) {
		logger->info("Exception: {}", e.what());
		return 1;
	}

	if (spine.loadModules(spine.moduleFileLocation)) {
		// We have to run this after loadModules because the config is provided by libmainline_config.
		bool spineReturn = spine.loadConfig("./modules/main.cfg");

		if (spineReturn) {
			logger->debug("{}: {}", "Main", "Running spine");
			spineReturn = spine.run();
		} else {
			logger->warn("{}: {}", "Main", "Config failed to load, shutting down");
		}

		//delete spine;
		return spineReturn ? 0 : 1;
	}
}