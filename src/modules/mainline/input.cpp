#include "modules/mainline/input.hpp"

void InputModule::setup() {
	out.setChannel(CHANNEL::Out);

	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_socketer->on("process_input", [&](const Message& message) {
		out.setChain(message.getChain());
		out.payload("ECHO " + message.payload());

		_socketer->sendMessage(out);

		return true;
	});

	Message moduleRunning(name());
	moduleRunning.setChannel(CHANNEL::Cmd);
	moduleRunning.payload("spine://module/loaded?name=" + name());
	_socketer->sendMessage(moduleRunning);
}

void InputModule::tick() {
}

InputModule* createModule(){return new InputModule;}
void destroyModule(InputModule* module) {delete module;}
