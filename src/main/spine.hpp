#ifndef H_SPINE
#define H_SPINE
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <dirent.h>
#include "main/module.hpp"

typedef struct SpineModule {
	void* module_so;
	Module_loader* loadModule;
	Module_unloader* unloadModule;
	Module* module;
	std::string moduleName;
} SpineModule;

class Spine : public Module {
	private:
		unsigned short moduleCount;
		std::vector<std::thread*> threads;
		std::vector<std::string> loadedModules;

		bool isModuleFile(std::string filename);
		int listDirectory(std::string directory, std::vector<std::string> &fileList);
		int openModuleFile(std::string fileName, SpineModule* spineModule);
		int resolveModuleFunctions(SpineModule* spineModule);
	public:
		Spine(unsigned short modules);
		std::string name();
		void run();
		bool loadModules(std::string directory);
		void close();
};

#endif // H_SPINE