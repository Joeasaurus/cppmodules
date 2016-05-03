#include "modules/mainline/output.hpp"

OutputModule::~OutputModule()
{
	this->closeSockets();
	_logger.log(name(), "Closed", true);
}

void OutputModule::setup() {
	_eventer.on("echoTime", [&](chrono::milliseconds) {
		message.payload("echoTime");
		sendMessage(message);
	}, chrono::milliseconds(1000), EventPriority::LOW);


	Command moduleRunning(name());
	moduleRunning.payload("module-loaded");
	sendMessage(moduleRunning);
}

void OutputModule::tick() {
	_eventer.emitTimedEvents();
}

OutputModule* createModule(){return new OutputModule;}
void destroyModule(OutputModule* module) {delete module;}
