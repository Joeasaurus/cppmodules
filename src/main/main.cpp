#include <cstdlib>
#include <string>
#include <iostream>
#include <dlfcn.h>
#include "main/module.hpp"

int main(int argc, char **argv) {
	void* module_so = dlopen("modules/base_module.mod", RTLD_NOW | RTLD_GLOBAL);
	if(!module_so){
		std::cerr << dlerror() << std::endl;
		exit(EXIT_FAILURE);
	}

    Module_loader* loadModule = (Module_loader*)dlsym(module_so, "loadModule");
    if (!loadModule) {
        exit(EXIT_FAILURE);
    }

    Module_unloader* unloadModule =
        (Module_unloader*)dlsym(module_so, "unloadModule");
    if (!unloadModule) {
        exit(EXIT_FAILURE);
    }

    Module* mod = loadModule();
    std::cout << mod->name() << std::endl;

  	unloadModule(mod);

    if (dlclose(module_so) != 0) {
        exit(EXIT_FAILURE);
    }

    return 0;
}
