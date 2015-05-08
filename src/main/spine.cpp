#include "main/spine.hpp"

Spine::Spine() {
	this->__info.name = "Spine";
	this->__info.author = "mainline";
	std::cout << "[Spine] Spine Open" << std::endl;
}

Spine::~Spine() {
	int position = 0;
	std::string closeMessage;
	// Here the Spine sends a message out on it's publish socket
	// Each message directs a 'close' message to a module registered
	//  in the Spine as 'loaded'
	// It then waits for the module to join. It relies on the module closing
	//  cleanly else it will lock up waiting.
	for (auto& modName : this->loadedModules) {
		closeMessage = modName + " close";
		this->sendMessage(SocketType::PUB, closeMessage);
		this->threads.at(position)->join();
		delete this->threads[position];
		position += 1;
	}
	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	this->closeSockets();
	std::cout << "[Spine] Closed" << std::endl;
}

std::vector<std::string> Spine::listModules(std::string directory) {
	std::vector<std::string> moduleFiles;

	// Here we list a directory and build a vector of files that match
	//  our platform specific dynamic lib extension (e.g., .so)
	try {
		boost::filesystem::directory_iterator startd(directory), endd;
		auto files = boost::make_iterator_range(startd, endd);

		for(boost::filesystem::path p : files){
			if (p.extension() == this->moduleFileExtension) {
				moduleFiles.push_back(p.string());
			}
		}
	} catch(boost::filesystem::filesystem_error e) {
		std::cerr << "[Spine] WARNING! Could not load modules!" << std::endl;
		std::cerr << "    - " << e.what() << std::endl;
	}

	return moduleFiles;
}

int Spine::openModuleFile(std::string moduleFile, SpineModule& spineModule) {
	// Here we use dlopen to load the dynamic library (that is, a compiled module)
	spineModule.module_so = dlopen(moduleFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (!spineModule.module_so) {
		std::cerr << dlerror() << std::endl;
		return 1;
	}
	return 0;
}

int Spine::resolveModuleFunctions(SpineModule& spineModule) {
	// Here we use dlsym to hook into exported functions of the module
	// One to load the module class and one to unload it
	spineModule.loadModule = (Module_loader*)dlsym(spineModule.module_so, "loadModule");
	if (!spineModule.loadModule) {
		 return 1;
	}

	spineModule.unloadModule = (Module_unloader*)dlsym(spineModule.module_so, "unloadModule");
	if (!spineModule.unloadModule) {
		return 2;
	}

	return 0;
}

bool Spine::loadModules(std::string directory) {
	// Here we gather a list of relevant module binaries
	//  and then create a new thread for each.
	// In the threads we load the binary and hook into it's exported functions.
	// We use the load function to create an instance of it's Module-derived class
	// We then set up an interface with the module using our input/output sockets
	//  which are a management Rep and Req socket for commands in and out
	//  and a chain-building pair of Pub and Sub sockets for message passing.
	std::vector<std::string> moduleFiles = this->listModules(directory);

	for (auto filename : moduleFiles)
	{
		std::thread* moduleThread = new std::thread([this, filename]() {
			int failure = 0;
			SpineModule spineModule;

			failure = this->openModuleFile(filename, spineModule);
			if (failure != 0) {
				exit(EXIT_FAILURE);
			}

			failure = this->resolveModuleFunctions(spineModule);
			if (failure != 0) {
				exit(EXIT_FAILURE);
			}

			spineModule.module = spineModule.loadModule();
			std::string moduleName = spineModule.module->name();

			spineModule.module->setSocketContext(this->inp_context);
			spineModule.module->openSockets();

			// Here we set the module to subscribe to it's name on it's subscriber socket
			// Then configure the module to connect it's publish output to the Spine input
			//  and it's mgmt output too
			spineModule.module->subscribe(moduleName);
			spineModule.module->notify(SocketType::PUB, "Spine");
			spineModule.module->notify(SocketType::MGM_OUT, "Spine");

			// Here we send a 'register' message to Spine via our now-connected MGM_OUT socket.
			// If the Spine succeeds in registering us, we will recieve a success message.
			std::string regMessage = "register " + moduleName;
			//TODO: This should be a callback!
			spineModule.module->sendMessage(SocketType::MGM_OUT, regMessage);

			// If the registration is a success,
			//  we call the module's 'run' function -- it's main run loop.
			bool moduleRun = spineModule.module->recvMessage<bool>(
				SocketType::MGM_OUT,
				[&](const std::string& regReply)
			{
				if (regReply == "success") {
					return spineModule.module->run();
				}
				return false;
			}, 3000);

			// Once here, the module's run function must have quit,
			//  so we should unload the binary. Then the thread will exit and close.
			spineModule.unloadModule(spineModule.module);
			// A failure here will (I think!) exit the thread badly!
			if (dlclose(spineModule.module_so) != 0) {
				exit(EXIT_FAILURE);
			}
		});
		// While the thread is running, we will wait for it's registration message
		// If it succeeds, we'll stick the thread object on our vector and carry on
		// If it fails, we'll wait for the thread to join, expecting it to be clean.
		if (this->registerModule()) {
			this->threads.push_back(moduleThread);
		} else {
			moduleThread->join();
			std::cerr << "[Spine] Failed to register module: " << filename << std::endl;
		}
	}

	return true;
}

bool Spine::registerModule() {
	// Here we wait to receive a "register" message from a module.
	// We stick it in the list of leaded modules,
	//  configure out publish socket to connect to their subscribe socket,
	//  and then return with a "success" message
	return this->recvMessage<bool>(SocketType::MGM_IN, [&](const std::string& regReply) {
		if (regReply != "__NULL_RECV_FAILED__") {
			std::vector<std::string> tokens;
			boost::split(tokens, regReply, boost::is_any_of(" "));
			if (tokens.at(0) == "register") {
				if (tokens.at(1) != "") {
					this->loadedModules.push_back(tokens.at(1));
					this->notify(SocketType::PUB, tokens.at(1));
					this->sendMessage(SocketType::MGM_IN, "success");
					return true;
				}
			}
		}
		return false;
	});
}

bool Spine::run() {
	try {
		while (true) {
			if (this->recvMessage<bool>(SocketType::PUB, [&](const std::string& messageText) {
				if (!messageText.empty()) {
					std::vector<std::string> messageTokens;
					boost::split(messageTokens, messageText, boost::is_any_of(" "));
					if (messageTokens.size() > 0) {
						// What was the 'command'?
						if (messageTokens.at(1) == "log") {
							std::cout << "[" << this->name() << "] Closing..." << std::endl;
							return true;
						}
					}
				}
				return false;
			}, 500)) {
				break;
			}
		}
		return true;
	} catch(...) {
		return false;
	}
	// Our function here just sleeps for a bit
	//  and then sends a close message to all modules ('Module' channel)
	for (int count = 0;count<3;count++) {
		std::cout << "Sleeping..." << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	this->sendMessage(SocketType::PUB, "Module close");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return true;
}

