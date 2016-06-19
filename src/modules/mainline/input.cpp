#include "modules/mainline/input.hpp"

void InputModule::setup() {
	out.setChannel(CHANNEL::Out);

	_socketer->on("process_input", [&](const Message& message) {
		out.setChain(message.getChain());
		out.payload("ECHO " + message.payload());

		_socketer->sendMessage(out);

		return true;
	});
}

void InputModule::tick() {
	_eventer->emitTimedEvents();
}

InputModule* createModule(){return new InputModule;}
void destroyModule(InputModule* module) {delete module;}
