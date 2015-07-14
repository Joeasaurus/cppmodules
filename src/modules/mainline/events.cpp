#include "modules/mainline/events.hpp"

EventsModule::EventsModule() {
	this->__info.name   = "Events";
	this->__info.author = "mainline";
}

EventsModule::~EventsModule() {
	this->closeSockets();
	this->logger->debug("{}: {}", this->name(), "Closed");
}

bool EventsModule::tickTimer() {
	this->timeNow = std::chrono::system_clock::now();
	return this->checkEventTimer();
}

bool EventsModule::checkEventTimer() {
	for (auto& event : this->events) {
		if (this->timeNow >= event.callTime) {
			if (!this->sendMessage(SocketType::PUB, event.owner, json::object{
				{ "eventName", event.name }
			})) {
				this->logger->debug("{}: {}", this->name(),
					"Failed to send event notification " +
					event.name + " to " + event.owner
				);
			};
			event.callTime = this->timeNow + std::chrono::seconds(event.interval);
		}
	}
	return true;
}

bool EventsModule::run() {
	bool runAgain = this->tickTimer();

	while (runAgain) {
		runAgain = this->pollAndProcess() ? this->tickTimer() : false;
	}
	return false;
}

bool EventsModule::process_message(const json::value& message, CatchState cought, SocketType sockT) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	return true;
}

EventsModule* loadModule() {
	return new EventsModule;
}

void unloadModule(EventsModule* module) {
	delete module;
}