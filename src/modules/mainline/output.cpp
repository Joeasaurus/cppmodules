#include "modules/mainline/output.hpp"

void OutputModule::setup() {
	message.setChannel(CHANNEL::Out);

	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_eventer.on("echoTime", [&](chrono::milliseconds) {
		message.payload("OutMessage");
		_socketer->sendMessage(message);
	}, chrono::milliseconds(1000), EventPriority::LOW);

	Message moduleRunning(name());
	moduleRunning.setChannel(CHANNEL::Cmd);
	moduleRunning.payload("spine://module/loaded?name=" + name());
	_socketer->sendMessage(moduleRunning);
}

void OutputModule::tick() {
	_eventer.emitTimedEvents();
}

OutputModule* createModule(){return new OutputModule;}
void destroyModule(OutputModule* module) {delete module;}
