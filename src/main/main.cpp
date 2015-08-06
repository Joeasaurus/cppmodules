#include "main/spine.hpp"

#include "main/segvhandler.hpp"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *    (SO boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

int main(int argc, char **argv) {
	signal(SIGSEGV, handler);
	bool spineReturn = false;
	// Configure the global logger first!!
	auto logger = Spine::createLogger(argc > 1 && strcmp(argv[1], "debug") == 0);

	logger->debug(ZMQ_VERSION);
	// Open the spine and pass it our logger
	Spine spine;
	try {
		spine.openSockets();
		if (spine.areSocketsValid()) {
			// We used to subscribe to ourselves for introspection
			// spine.subscribe(spine.name());
			if (spine.loadModules(spine.moduleFileLocation)) {
				// // We have to run this after loadModules because the config is provided by libmainline_config.
				this_thread::sleep_for(chrono::seconds(3));
				spine.sendMessage(SocketType::PUB, "Modules", json::object{
					{ "message", "YOOO" }
				});
				spineReturn = spine.loadConfig("./modules/main.cfg");
				if (spineReturn) {
					logger->debug("{}: {}", "Main", "Running spine");
					spineReturn = spine.run();
				} else {
					logger->warn("{}: {}", "Main", "Config failed to load, shutting down");
					spineReturn = false;
				}
			}
		} else {
			logger->debug("NOT VALID");
		}
	} catch (const zmq::error_t &e) {
		logger->info("Exception: {}", e.what());
		spineReturn = false;
	}
	return spineReturn ? 0 : 1;
}