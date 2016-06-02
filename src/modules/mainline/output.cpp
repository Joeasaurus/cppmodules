#include "modules/mainline/output.hpp"

void OutputModule::setup() {

	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_eventer.on("echoTime", [&](chrono::milliseconds) {
		message.payload("OutMessage");
		_socketer->sendMessage(message);
	}, chrono::milliseconds(1000), EventPriority::LOW);

	Command moduleRunning(name());
	moduleRunning.payload("module-loaded");
	_socketer->sendMessage(moduleRunning);
}

void OutputModule::tick() {
	_eventer.emitTimedEvents();
}

OutputModule* createModule(){return new OutputModule;}
void destroyModule(OutputModule* module) {delete module;}
