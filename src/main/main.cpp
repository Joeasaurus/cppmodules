#include "main/module.hpp"
#include "main/spine.hpp"

int main(int argc, char **argv) {
	zmq::context_t spineCtx(1);

	Spine spine;
	try {
		spine.setSocketContext(&spineCtx);
		spine.openSockets();
	} catch (const zmq::error_t &e) {
		std::cout << e.what() << std::endl;
	}

    spine.loadModules("./modules");
 	spine.run();

    return 0;
}