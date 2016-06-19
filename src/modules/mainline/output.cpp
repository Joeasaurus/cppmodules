#include "modules/mainline/output.hpp"

using namespace cppm::exceptions::uri;

void OutputModule::setup() {
	hookOut();
}

void OutputModule::hookOut() {
	message.setChannel(CHANNEL::Out);
	_eventer->on("echoTime", [&](chrono::milliseconds) {
		message.payload("OutMessage");
		_socketer->sendMessage(message);
	}, chrono::milliseconds(1000), EventPriority::LOW);
}

void OutputModule::tick() {
	_eventer->emitTimedEvents();
}

OutputModule* createModule(){return new OutputModule;}
void destroyModule(OutputModule* module) {delete module;}
