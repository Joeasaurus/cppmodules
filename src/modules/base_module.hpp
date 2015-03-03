#ifndef H_BASE_MODULE
#define H_BASE_MODULE
#include "main/module.hpp"

class BaseModule : public Module {
	public:
		BaseModule();
		~BaseModule();
		std::string name();
		void run();
};

// Init/Del functions.
extern "C" BaseModule* loadModule();
extern "C" void unloadModule(BaseModule* module);

#endif //H_BASE_MODULE
