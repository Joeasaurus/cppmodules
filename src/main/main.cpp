#include "main/spine.hpp"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *    (So boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

using namespace cppm;
using namespace std;

int main(int argc, char **argv) {
	Logger logger;

	// This is a quick fix for debug logging by managing argv ourselves
	// We'll move to an option parser later
	logger.setDebug(argc > 1 && strcmp(argv[1], "debug") == 0);

	Spine spine;
	// We used to subscribe to ourself for introspection
	if (spine.loadModules()) {
		while (true) {
			spine.pollAndProcess();
		}
	}
	return false;
}