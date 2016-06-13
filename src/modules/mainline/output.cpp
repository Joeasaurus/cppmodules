#include "modules/mainline/output.hpp"

using namespace cppm::exceptions::uri;

void OutputModule::setup() {
	message.setChannel(CHANNEL::Out);

	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		MUri uri(message.payload());

		try {
			if (uri.domain() == "module" && uri.command() == "loaded")
			 	if (uri.param("success").front() == "true") registered = true;
		} catch (ParamNotFound& e) {
			_logger.err(name(), e.what());
			return false;
		}
		return true;
	});

	_eventer.on("echoTime", [&](chrono::milliseconds) {
		message.payload("OutMessage");
		_socketer->sendMessage(message);

	}, chrono::milliseconds(1000), EventPriority::LOW);

	_eventer.on("send-registration", [&] {

		Message moduleRunning(name());
		moduleRunning.setChannel(CHANNEL::Cmd);
		moduleRunning.payload("spine://module/loaded?name=" + name());
		_socketer->sendMessage(moduleRunning);

	});

	_eventer.emit("send-registration");
}

void OutputModule::tick() {
	if (! registered) _eventer.emit("send-registration");
	_eventer.emitTimedEvents();
}

OutputModule* createModule(){return new OutputModule;}
void destroyModule(OutputModule* module) {delete module;}
