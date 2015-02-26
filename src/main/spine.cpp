#include "main/spine.hpp"

Spine::Spine(unsigned short maxModules) {
	this->__info.name = "main_Spine";
	this->__info.author = "mainline";
	this->moduleCount = maxModules;
	std::cout << "[Spine] Spine Open" << std::endl;
}

// Spine::~Spine() {
// 	if (!this->closedModules) {
// 		this->close();
// 	}
// 	std::cout << "[Spine] Spine Closed" << std::endl;
// }

std::string Spine::name() {
	return this->__info.name;
}

int Spine::listDirectory(std::string directory, std::vector<std::string> &fileList) {
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(directory.c_str())) == NULL) {
		std::cout << "Error(" << errno << ") opening " << directory << std::endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		std::string dirName = dirp->d_name;
		if (dirName != "." && dirName != "..") {
			fileList.push_back(static_cast<std::string>(dirp->d_name));
		}
	}
	closedir(dp);
	return 0;
}

int Spine::openModuleFile(std::string moduleFile, SpineModule* spineModule) {
	spineModule->module_so = dlopen(moduleFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (!spineModule->module_so) {
		std::cerr << dlerror() << std::endl;
		return 1;
	}
	return 0;
}

int Spine::resolveModuleFunctions(SpineModule* spineModule) {
	spineModule->loadModule = (Module_loader*)dlsym(spineModule->module_so, "loadModule");
	if (!spineModule->loadModule) {
		 return 1;
	}

	spineModule->unloadModule = (Module_unloader*)dlsym(spineModule->module_so, "unloadModule");
	if (!spineModule->unloadModule) {
		return 2;
	}

	return 0;
}

bool Spine::loadModules(std::string directory) {
	int failure = 0;

	std::vector<std::string> moduleFiles;
	this->listDirectory(directory, moduleFiles);

	for (std::vector<std::string>::iterator iter = moduleFiles.begin();
			iter != moduleFiles.end(); ++iter)
	{
		SpineModule* spineModule = new SpineModule();
		std::string location = directory + "/" + *iter;

		failure = this->openModuleFile(location, spineModule);
		if (failure != 0) {
			exit(EXIT_FAILURE);
		}

		failure = this->resolveModuleFunctions(spineModule);
		if (failure != 0) {
			exit(EXIT_FAILURE);
		}

		spineModule->module = spineModule->loadModule();
		spineModule->moduleName = spineModule->module->name();

		spineModule->module->setSocketContext(this->ipc_context);
		spineModule->module->openSockets(spineModule->moduleName);

		this->modules.push_back(spineModule);
	}

	return true;
}

void Spine::sendMessage(std::string module, std::string message) {
	std::cout <<"[Spine] " << message << " " << this->modules[0]->module->name() << std::endl;
}

void Spine::close() {
	for (std::vector<SpineModule*>::iterator iter = this->modules.begin();
			iter != this->modules.end(); ++iter)
	{
		Module* mod = (*iter)->module;
		mod->close();
		mod->closeSockets();
		(*iter)->unloadModule(mod);
		if (dlclose((*iter)->module_so) != 0) {
			exit(EXIT_FAILURE);
		}
	}
	this->ipc_manage->close();
	std::cout << "[Spine] Closed" << std::endl;
}

