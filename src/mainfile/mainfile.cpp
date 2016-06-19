#include "main/spine/spine.hpp"
#include "main/interfaces/logger.hpp"
#include <iostream>

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *	(So boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

using namespace cppm;
using namespace cppm::interfaces;
using namespace std;

int main(int argc, char **argv) {

	Logger logger;
	logger.setDebug(true);
	string modPath = "NONE";

	// This is a quick fix for debug logging by managing argv ourselves
	// We'll move to an option parser later
	if (argc > 1) {
		modPath = argv[1];
	}

	if (modPath == "NONE") {
		logger.err("[Main]", "You forgot to supply a modules directory!");
		return 1;
	}

	Spine spine;

	if (spine.loadModules(modPath)) {
		while (spine.isRunning()) {
			try {
				spine.polltick();
			} catch (exception& e) {
				logger.err("Main", string("CAUGHT POLLTICK OF SPINE ") + e.what());
			}
		}
		return 0;
	}

	return 1;
}
