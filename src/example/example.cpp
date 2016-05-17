#include "main/spine.hpp"

/* NOTES
 * To save space in modules, spine should provide functions for loading files?
 *	(So boost-filesystem + system only need linking once)
 * Change module loading so we use author + name instead of relying on the filename
 */

using namespace cppm;
using namespace std;

int main(int argc, char **argv) {

	Logger logger;
	string modPath = "NONE";
	auto context = make_shared<zmq::context_t>(1);

	// This is a quick fix for debug logging by managing argv ourselves
	// We'll move to an option parser later
	if (argc > 1) {

		if (strcmp(argv[1], "debug") == 0) {
			logger.setDebug(true);
		} else {
			modPath = argv[1];
		}

		if (argc > 2 && strcmp(argv[1], "debug") == 0) {
			logger.setDebug(true);
			modPath = argv[2];
		}
	}

	if (modPath == "NONE") {
		logger.err("[Main]", "You forgot to supply a modules directory!");
		return 1;
	}

	Spine spine(context);

	if (spine.loadModules(modPath)) {
		while (spine.isRunning()) {
			try {
				spine.polltick();
			} catch (exception& e) {
				cout << "CAUGHT POLLTICK IN MAIN " << e.what() << endl;
			}
		}
		return 0;
	}

	return 1;
}
