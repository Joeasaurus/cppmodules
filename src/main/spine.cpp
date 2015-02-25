#include <dirent.h>
#include "main/spine.hpp"

Spine::Spine(unsigned short maxModules) {
	this->moduleCount = maxModules;
	std::cout << "[Spine] Spine Open" << std::endl;
}

Spine::~Spine() {
	if (!this->closedModules) {
		this->close();
	}
	std::cout << "[Spine] Spine Closed" << std::endl;
}

bool Spine::loadModules(std::string directory) {
	SpineModule* spineModule = new SpineModule();
	spineModule->module_so = dlopen("modules/base_module.so", RTLD_NOW | RTLD_GLOBAL);
	if(!spineModule->module_so){
		std::cerr << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

    spineModule->loadModule = (Module_loader*)dlsym(spineModule->module_so, "loadModule");
    if (!spineModule->loadModule) {
        exit(EXIT_FAILURE);
    }

    spineModule->unloadModule =
        (Module_unloader*)dlsym(spineModule->module_so, "unloadModule");
    if (!spineModule->unloadModule) {
        exit(EXIT_FAILURE);
    }

    spineModule->module = spineModule->loadModule();

    this->modules.push_back(*spineModule);
    return true;
}

void Spine::sendMessage(std::string module, std::string message) {
	std::cout <<"[Spine] " << message << " " << this->modules[0].module->name() << std::endl;
}

void Spine::close() {
   	for (unsigned short i = 0; i<this->modules.size();i++) {
		this->modules[i].unloadModule(this->modules[i].module);
		if (dlclose(this->modules[i].module_so) != 0) {
			exit(EXIT_FAILURE);
    	}
	}
	this->closedModules = true;
}

