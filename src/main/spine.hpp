#ifndef H_SPINE
#define H_SPINE
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <dlfcn.h>

#include "main/module.hpp"

typedef struct SpineModule {
	void* module_so;
	Module_loader* loadModule;
	Module_unloader* unloadModule;
	Module* module;
} SpineModule;

class Spine : public Module {
	private:
		std::vector<std::thread*> threads;
		std::vector<std::string> loadedModules;

		bool isModuleFile(std::string filename);
		std::vector<std::string> listModules(std::string directory);
		int openModuleFile(std::string fileName, SpineModule* spineModule);
		int resolveModuleFunctions(SpineModule* spineModule);
	public:
		Spine();
		~Spine();
		std::string name();
		void run();
		bool loadModules(std::string directory);
};

#endif // H_SPINE