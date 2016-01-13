#include "main/spine.hpp"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *    (So boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

using namespace cppm;

int main(int argc, char **argv) {
	Logger logger;

	// This is a quick fix for debug logging by managing argv ourselves
	// We'll move to an option parser later
	logger.setDebug(argc > 1 && strcmp(argv[1], "debug") == 0);

	Spine spine;
	// We used to subscribe to ourself for introspection
	//spine.subscribe(spine.name());
	if (spine.loadModules()) {
		// We have to run this after loadModules because the config is provided by libmainline_config.
		//if (spine.loadConfig(spine.moduleFileLocation + "/main.cfg")) {
			spine.run();
		//} else {
			//logger.getLogger()->error("{}: {}", "Main", "Config failed to load, shutting down");
		//}
	}
	return false;
}