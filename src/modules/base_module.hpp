#include "main/module.hpp"

#ifndef H_BASE_MODULE
#define H_BASE_MODULE

class BaseModule : public Module {
	public:
		BaseModule();
		virtual ~BaseModule();
		virtual std::string name();
};

// Init/Del functions.
extern "C" BaseModule* loadModule();
extern "C" void unloadModule(BaseModule* module);

#endif //H_BASE_MODULE
