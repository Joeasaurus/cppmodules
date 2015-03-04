#ifndef H_CORE_MODULE
#define H_CORE_MODULE
#include "main/module.hpp"

class CoreModule : public Module {
	public:
		CoreModule();
		~CoreModule();
		std::string name();
		void run();
};

// Init/Del functions.
extern "C" CoreModule* loadModule();
extern "C" void unloadModule(CoreModule* module);

#endif //H_CORE_MODULE
