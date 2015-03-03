#include "main/spine.hpp"

Spine::Spine(unsigned short maxModules) {
	this->__info.name = "Spine";
	this->__info.author = "mainline";

	this->moduleCount = maxModules;
	std::cout << "[Spine] Spine Open" << std::endl;
}

std::string Spine::name() {
	return this->__info.name;
}

// This example is ripped from stackoverflow, I don't want to pull in boost just for this!
bool Spine::isModuleFile(std::string filename) {
	size_t last_dot_offset = filename.rfind('.');
    // This assumes your directory separators are either \ or /
    size_t last_dirsep_offset = std::max( filename.rfind('\\'), filename.rfind('/') );

    // no dot = no extension
    if( last_dot_offset == std::string::npos )
        return false;

    // directory separator after last dot = extension of directory, not file.
    // for example, given C:\temp.old\file_that_has_no_extension we should return "" not "old"
    if( (last_dirsep_offset != std::string::npos) && (last_dirsep_offset > last_dot_offset) )
        return false;

    if (filename.substr( last_dot_offset + 1 ) == "so") {
    	return true;
    }
    return false;
}

int Spine::listDirectory(std::string directory, std::vector<std::string> &fileList) {
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(directory.c_str())) == NULL) {
		std::cout << "Error(" << errno << ") opening " << directory << std::endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		std::string dirName = static_cast<std::string>(dirp->d_name);
		if (this->isModuleFile(dirName)) {
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
	std::vector<std::string> moduleFiles;
	this->listDirectory(directory, moduleFiles);

	for (std::vector<std::string>::iterator iter = moduleFiles.begin();
			iter != moduleFiles.end(); ++iter)
	{
		std::string filename = static_cast<std::string>(*iter);
		std::thread* moduleThread = new std::thread([this, directory, filename]() {
			int failure = 0;
			SpineModule* spineModule = new SpineModule();
			std::string location = directory + "/" + filename;

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

			spineModule->module->setSocketContext(this->inp_context);
			spineModule->module->openSockets();

			spineModule->module->subscribe(spineModule->module->name());
			spineModule->module->connectOutput("publish", "Spine");
			spineModule->module->connectOutput("manage", "Spine");

			std::string regMessage = "register " + spineModule->moduleName;
			spineModule->module->sendMessage("manage-out", regMessage, "");
			std::string regReply = spineModule->module->recvMessage("manage-out");
			if (regReply == "success") {
				spineModule->module->run();
			}

			spineModule->unloadModule(spineModule->module);
			if (dlclose(spineModule->module_so) != 0) {
				exit(EXIT_FAILURE);
			}
		});
		std::string regMessage = this->recvMessage("manage-in");
		std::vector<std::string> tokens;
		this->splitString(regMessage, ' ', tokens);
		if (tokens.at(0) == "register") {
			this->threads.push_back(moduleThread);
			this->loadedModules.push_back(tokens.at(1));
			this->connectOutput("publish", tokens.at(1));
			this->sendMessage("manage-in", "success", "");
		}
	}

	return true;
}

void Spine::close() {
	int position = 0;
	for (auto& modName : this->loadedModules) {
		this->sendMessage("publish", "close", modName);
		this->threads.at(position)->join();
		position += 1;
	}
	this->closeSockets();
	std::cout << "[Spine] Closed" << std::endl;
}

void Spine::run() {
    //    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (int count = 0;count<3;count++) {
		std::cout << "Sleeping..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	this->sendMessage("publish", "close", "Module");
	std::this_thread::sleep_for(std::chrono::seconds(5));
}

