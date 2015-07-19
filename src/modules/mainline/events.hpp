#pragma once
// Common
#include "main/module.hpp"
// Module Specific

typedef struct Event {
	std::string owner;
	std::string name;
	short interval;
	std::chrono::system_clock::time_point callTime;
} Event;

class EventsModule : public Module {
	public:
		EventsModule();
		~EventsModule();
		bool run();
		bool process_message(const json::value& message, CatchState cought, SocketType sockT);
	private:
		std::chrono::system_clock::time_point timeNow;
		std::vector<Event> events;
		bool tickTimer();
		bool checkEventTimer();
};

// Init/Del functions.
extern "C" EventsModule* loadModule();
extern "C" void unloadModule(EventsModule* module);
