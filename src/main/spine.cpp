#include "main/spine.hpp"

Spine::Spine() {
	this->__info.name = "Spine";
	this->__info.author = "mainline";
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
		this->sendMessage(SocketType::PUB, modName, json::object{
			{ "command", "close" }
		});
		this->logger->debug("{}: {}", this->name(), "Joining " + modName);
		this->threads.at(position)->thread_pointer->join();
		this->logger->debug("{}: {}", this->name(), "Joined");
		this->threads.at(position)->module.unloadModule(this->threads.at(position)->module.module);
		if (dlclose(this->threads.at(position)->module.module_so) != 0) {
			this->logger->error("{}: {}", this->name(), "Could not dlclose module file");
		}
		delete this->threads[position]->thread_pointer;
		delete this->threads[position];
		position += 1;
	}
	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	this->closeSockets();
	logger->info("{}: {}", this->name(), "Closed");
}

std::set<std::string> Spine::listModules(const std::string& directory) {
	std::set<std::string> moduleFiles;

	// Here we list a directory and build a vector of files that match
	//  our platform specific dynamic lib extension (e.g., .so)
	try {
		boost::filesystem::recursive_directory_iterator startd(directory), endd;
		auto files = boost::make_iterator_range(startd, endd);

		for(boost::filesystem::path p : files){
			if (p.extension() == this->moduleFileExtension &&
				!boost::filesystem::is_directory(p)
			) {
				moduleFiles.insert(p.string());
			}
		}
	} catch(boost::filesystem::filesystem_error e) {
		this->logger->warn("{}: {}", this->name(), "WARNING! Could not load modules!");
		this->logger->warn("{}: {}{}", this->name(), "    - ", e.what());
	}

	return moduleFiles;
}

bool Spine::openModuleFile(const std::string& moduleFile, SpineModule& spineModule) {
	// Here we use dlopen to load the dynamic library (that is, a compiled module)
	spineModule.module_so = dlopen(moduleFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (!spineModule.module_so) {
		this->logger->error("{}: {}", this->name(), dlerror());
		return false;
	}
	return true;
}

int Spine::resolveModuleFunctions(SpineModule& spineModule) {
	// Here we use dlsym to hook into exported functions of the module
	// One to load the module class and one to unload it
	spineModule.loadModule = (Module_loader*)dlsym(spineModule.module_so, "loadModule");
	if (!spineModule.loadModule) {
		this->logger->error("{}: {}", this->name(), dlerror());
		return 1;
	}
	this->logger->debug("{}: {}", this->name(), "Resolved loadModule");

	spineModule.unloadModule = (Module_unloader*)dlsym(spineModule.module_so, "unloadModule");
	if (!spineModule.unloadModule) {
		return 2;
	}
	this->logger->debug("{}: {}", this->name(), "Resolved unloadModule");

	return 0;
}

bool Spine::loadModule(const std::string& filename) {
	SpineThread* spineThread = new SpineThread;
	SpineModule spineModule;
	this->logger->debug("{}: {}", this->name(), "Loading " + boost::filesystem::basename(filename) + "...");
	// In the threads we load the binary and hook into it's exported functions.
	// We use the load function to create an instance of it's Module-derived class
	// We then set up an interface with the module using our input/output sockets
	//  which are a management Rep and Req socket for commands in and out
	//  and a chain-building pair of Pub and Sub sockets for message passing.
	// Now we run it's run() function in a new thread and push that on to our list.
	if (! this->openModuleFile(filename, spineModule)) {
		this->logger->error("{}: {}", this->name(), "Failiure " + filename + "..");
		delete spineThread;
		return false;
	}
	this->logger->debug("{}: {}", this->name(), "Opened file " + filename + "..");

	int failure = 0;
	failure = this->resolveModuleFunctions(spineModule);
	if (failure != 0) {
		this->logger->error("{}: {}", this->name(), "Failiure " + filename + "..");
		delete spineThread;
		return false;
	}
	this->logger->debug("{}: {}", this->name(), "Functions resolved");

	spineModule.module = spineModule.loadModule();
	std::string moduleName = spineModule.module->name();

	spineModule.module->setLogger(this->logger);
	spineModule.module->setSocketContext(this->inp_context);
	spineModule.module->openSockets();

	// Here we set the module to subscribe to it's name on it's subscriber socket
	// Then configure the module to connect it's publish output to the Spine input
	//  and it's mgmt output too
	spineModule.module->subscribe("Modules");
	spineModule.module->subscribe(moduleName);
	spineModule.module->notify(SocketType::PUB, "Spine");
	spineModule.module->notify(SocketType::MGM_OUT, "Spine");
	this->logger->debug("{}: {}", this->name(), "Sockets Registered for: " + moduleName + "!");

	this->notify(SocketType::PUB, moduleName);
	this->notify(SocketType::MGM_OUT, moduleName);
	this->loadedModules.insert(moduleName);
	this->logger->info("{}: {}", this->name(), "Registered module: " + moduleName + "!");

	spineThread->thread_pointer = new std::thread([&spineModule] () {
		spineModule.module->run();
	});

	spineThread->module = spineModule;
	this->threads.push_back(spineThread);

	return true;
}

bool Spine::loadModules(const std::string& directory) {
	// Here we gather a list of relevant module binaries
	//  and then call the load function on each path
	std::set<std::string> moduleFiles = this->listModules(directory);

	// Before we iterate for loading, we will try loading a few required modules
	//  in our pre-defined order. If any fail, we don't load the rest.
	boost::filesystem::path moduleLoc(this->moduleFileLocation);
	boost::filesystem::path configModule(moduleLoc /
							("libmainline_config" + this->moduleFileExtension));

	bool configLoaded = this->loadModule(configModule.string());
	if (configLoaded) {
		moduleFiles.erase(configModule.string());

	} else {
		return false;
	}

	for (auto filename : moduleFiles)
	{
		// We should check error messages here better!
		this->loadModule(filename);
	}

	return true;
}

bool Spine::loadConfig(std::string location) {
	this->sendMessage(SocketType::MGM_OUT, "Config", json::object{
		{ "command", "load" },
		{ "file", location }
	});
	return true;
}

bool Spine::run() {
	try {
		// Our function here just sleeps for a bit
		//  and then sends a close message to all modules ('Module' channel)
		for (int count = 0;count<3;count++) {
			this->pollAndProcess();
			this->logger->info("{}: {}", this->name(), "Sleeping...");
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		// Lets make sure the close command works!
		this->logger->info("{}: {}", this->name(), "Closing 'Modules'");
		this->sendMessage(SocketType::PUB, "Modules", json::object{
			{ "command", "close" }
		});
		std::this_thread::sleep_for(std::chrono::seconds(5));
		this->logger->info("{}: {}", this->name(), "All modules closed");
		return true;
	} catch(...) {
		this->logger->info("EXIT");
		return false;
	}
}

bool Spine::process_message(const json::value& message, CatchState cought) {
	this->logger->debug("{}: {}", this->name(), stringify(message));
	return true;
}

