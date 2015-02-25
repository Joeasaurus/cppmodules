#ifndef H_SPINE
#define H_SPINE
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>
#include "module.hpp"

typedef struct SpineModule {
	void* module_so;
	Module_loader* loadModule;
	Module_unloader* unloadModule;
	Module* module;
} SpineModule;

class Spine {
	private:
		unsigned short moduleCount;
		std::vector<SpineModule> modules;
		bool closedModules = false;
	public:
		Spine(unsigned short modules);
		~Spine();
		bool loadModules(std::string directory);
		void sendMessage(std::string module, std::string message);
		void close();
};

#endif