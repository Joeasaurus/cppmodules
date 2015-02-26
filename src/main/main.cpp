#include "main/module.hpp"
#include "main/spine.hpp"

int main(int argc, char **argv) {
	zmq::context_t ipc_context(1);

    Spine* spine = new Spine(1);

    spine->setSocketContext(&ipc_context);
	spine->openSockets("Spint");

    spine->loadModules("./modules");
    spine->sendMessage("BaseModule", "Initialise");

    spine->close();
    delete spine;

    ipc_context.close();
    return 0;
}