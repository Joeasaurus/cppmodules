#ifndef H_SPINE
#define H_SPINE
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <dirent.h>
#include "module.hpp"

typedef struct SpineModule {
	void* module_so;
	Module_loader* loadModule;
	Module_unloader* unloadModule;
	Module* module;
	std::string moduleName;
} SpineModule;

class Spine : public Module {
	private:
		ModuleInfo __info;
		unsigned short moduleCount;
		std::vector<SpineModule*> modules;

		int listDirectory(std::string directory, std::vector<std::string> &fileList);
		int openModuleFile(std::string fileName, SpineModule* spineModule);
		int resolveModuleFunctions(SpineModule* spineModule);
	public:
		Spine(unsigned short modules);
		std::string name();
		bool loadModules(std::string directory);
		void sendMessage(std::string module, std::string message);
		void close();
};

#endif // H_SPINE