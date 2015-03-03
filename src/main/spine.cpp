#include "main/spine.hpp"

Spine::Spine() {
	this->__info.name = "Spine";
	this->__info.author = "mainline";
	std::cout << "[Spine] Spine Open" << std::endl;
}

Spine::~Spine() {
	int position = 0;
	for (auto& modName : this->loadedModules) {
		this->sendMessage(SocketType::PUB, "close", modName);
		this->threads.at(position)->join();
		position += 1;
	}
	this->closeSockets();
	std::cout << "[Spine] Closed" << std::endl;
}

std::string Spine::name() {
	return this->__info.name;
}

std::vector<std::string> Spine::listModules(std::string directory) {
	std::vector<std::string> moduleFiles;
	boost::filesystem::directory_iterator startd(directory), endd;
	auto files = boost::make_iterator_range(startd, endd);

	for(boost::filesystem::path p : files){
		if (p.extension() == ".so") {
			moduleFiles.push_back(p.string());
		}
	}
	return moduleFiles;
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
	std::vector<std::string> moduleFiles = this->listModules(directory);

	for (auto filename : moduleFiles)
	{
		std::thread* moduleThread = new std::thread([this, filename]() {
			int failure = 0;
			SpineModule* spineModule = new SpineModule();

			failure = this->openModuleFile(filename, spineModule);
			if (failure != 0) {
				exit(EXIT_FAILURE);
			}

			failure = this->resolveModuleFunctions(spineModule);
			if (failure != 0) {
				exit(EXIT_FAILURE);
			}

			spineModule->module = spineModule->loadModule();
			std::string moduleName = spineModule->module->name();

			spineModule->module->setSocketContext(this->inp_context);
			spineModule->module->openSockets();

			spineModule->module->subscribe(moduleName);
			spineModule->module->connectOutput(SocketType::PUB, "Spine");
			spineModule->module->connectOutput(SocketType::MGM_OUT, "Spine");

			std::string regMessage = "register " + moduleName;
			spineModule->module->sendMessage(SocketType::MGM_OUT, regMessage, "");
			std::string regReply = spineModule->module->recvMessage(SocketType::MGM_OUT);
			if (regReply == "success") {
				spineModule->module->run();
			}

			spineModule->unloadModule(spineModule->module);
			if (dlclose(spineModule->module_so) != 0) {
				exit(EXIT_FAILURE);
			}
		});
		std::string regMessage = this->recvMessage(SocketType::MGM_IN);
		std::vector<std::string> tokens;
		boost::split(tokens, regMessage, boost::is_any_of(" "));
		if (tokens.at(0) == "register") {
			this->threads.push_back(moduleThread);
			this->loadedModules.push_back(tokens.at(1));
			this->connectOutput(SocketType::PUB, tokens.at(1));
			this->sendMessage(SocketType::MGM_IN, "success", "");
		}
	}

	return true;
}

void Spine::run() {
	//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (int count = 0;count<3;count++) {
		std::cout << "Sleeping..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	this->sendMessage(SocketType::PUB, "close", "Module");
	std::this_thread::sleep_for(std::chrono::seconds(5));
}

