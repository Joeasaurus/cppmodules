#include "main/spine.hpp"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *    (So boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

int main(int argc, char **argv) {
	bool spineReturn = false;

	// Open the spine and get the logger it creates
	Spine spine(argc > 1 && strcmp(argv[1], "debug") == 0);
	auto logger = spine.getLogger();

	try {
		spine.openSockets();
		if (spine.areSocketsValid()) {
			// We used to subscribe to ourselves for introspection
			//spine.subscribe(spine.name());
			if (spine.loadModules(spine.moduleFileLocation)) {
				// We have to run this after loadModules because the config is provided by libmainline_config.
				spineReturn = spine.loadConfig(spine.moduleFileLocation + "/main.cfg");
				if (spineReturn) {
					logger->debug("{}: {}", "Main", "Running spine");
					spineReturn = spine.run();
				} else {
					logger->warn("{}: {}", "Main", "Config failed to load, shutting down");
					spineReturn = false;
				}
			}
		} else {
			logger->debug("{}: {}", "Main", "Spine sockets are invalid, quitting..");
			spineReturn = false;
		}
	} catch (const zmq::error_t &e) {
		logger->info("Exception: {}", e.what());
		spineReturn = false;
	}
	return spineReturn ? 0 : 1;
}