#include "main/spine.hpp"

namespace cppm {

Spine::Spine(bool debugLogLevel) : Module("Spine", "Joe Eaves") {
	//TODO: Is there a lot of stuff that could throw here? It needs looking at
	this->logger = Spine::createLogger(debugLogLevel);
	this->inp_context = new zmq::context_t(1);
	this->openSockets();
}

Spine::~Spine() {
	int position = 0;
	string closeMessage;
	// Here the Spine sends a message out on it's publish socket
	// Each message directs a 'close' message to a module registered
	//  in the Spine as 'loaded'
	// It then waits for the module to join. It relies on the module closing
	//  cleanly else it will lock up waiting.
	Message wMsg(this->name(), "");
	wMsg["data"]["command"] = "close";
	for_each(m_modules.begin(), m_modules.end(), [&](SpineModule& mod) {
		wMsg["destination"] = mod.moduleName;
		this->sendMessage(SocketType::PUB, wMsg);
		this->logger->debug(this->nameMsg("Joining " + mod.moduleName));

		if (m_threads[position].joinable()) {
			m_threads[position].join();
			this->logger->debug(this->nameMsg("Joined"));
		}

		m_modules[position].destroyModule(m_modules[position].module);
		if (dlclose(m_modules[position].module_so) != 0) {
			this->logger->error(this->nameMsg("Could not dlclose module file"));
		}

		position++;
	});

	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	this->closeSockets();
	this->logger->info(this->nameMsg("Closed"));
}

const shared_ptr<spdlog::logger> Spine::createLogger(bool debug) {
	string loggerName = "SpineLogger";
	spdlog::set_async_mode(1048576); //queue size must be power of 2

	// This is a quick fix for debug logging by managing argv ourselves
	// We'll move to an option parser later
	spdlog::set_level(debug ? sllevel::debug : sllevel::info);

	// Create a stdout logger, multi threaded
	shared_ptr<spdlog::logger> newLogger;
	try {
		newLogger = spdlog::stdout_logger_mt(loggerName);
	} catch (const spdlog::spdlog_ex) {
		newLogger = spdlog::get(loggerName);
	}

	newLogger->set_pattern("[%T.%e] [%l] %v"); // Custom format

	return newLogger;
}

set<string> Spine::listModules(const string& directory) {
	set<string> moduleFiles;

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
	} catch(boost::filesystem::filesystem_error& e) {
		this->logger->warn(this->nameMsg("WARNING! Could not load modules!"));
		this->logger->warn(this->nameMsg("    - " + static_cast<string>(e.what())));
	}

	return moduleFiles;
}

bool Spine::openModuleFile(const string& moduleFile, SpineModule& spineModule) {
	// Here we use dlopen to load the dynamic library (that is, a compiled module)
	spineModule.module_so = dlopen(moduleFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (!spineModule.module_so) {
		this->logger->error(this->nameMsg(dlerror()));
		return false;
	}
	return true;
}

int Spine::resolveModuleFunctions(SpineModule& spineModule) {
	// Here we use dlsym to hook into exported functions of the module
	// One to load the module class and one to unload it
	spineModule.createModule = (Module_ctor*)dlsym(spineModule.module_so, "createModule");
	if (!spineModule.createModule) {
		this->logger->error(this->nameMsg(dlerror()));
		return 1;
	}
	this->logger->debug(this->nameMsg("Resolved loadModule"));

	spineModule.destroyModule = (Module_dctor*)dlsym(spineModule.module_so, "destroyModule");
	if (!spineModule.destroyModule) {
		return 2;
	}
	this->logger->debug(this->nameMsg("Resolved unloadModule"));

	return 0;
}

bool Spine::loadModule(const string& filename) {
	SpineModule spineModule;
	this->logger->debug(this->nameMsg("Loading " + boost::filesystem::basename(filename) + "..."));

	// In the threads we load the binary and hook into it's exported functions.
	// We use the load function to create an instance of it's Module-derived class
	// We then set up an interface with the module using our input/output sockets
	//  which are a management Rep and Req socket for commands in and out
	//  and a chain-building pair of Pub and Sub sockets for message passing.
	// Now we run it's run() function in a new thread and push that on to our list.
	if (! this->openModuleFile(filename, spineModule)) {
		this->logger->error(this->nameMsg("Failiure " + boost::filesystem::basename(filename) + ".."));
		return false;
	}
	this->logger->debug(this->nameMsg("Opened file " + boost::filesystem::basename(filename) + ".."));

	int failure = 0;
	failure = this->resolveModuleFunctions(spineModule);
	if (failure != 0) {
		this->logger->error(this->nameMsg("Failiure " + boost::filesystem::basename(filename) + ".."));
		return false;
	}
	this->logger->debug(this->nameMsg("Functions resolved"));

	spineModule.module = spineModule.createModule();
	spineModule.moduleName = spineModule.module->name();

	spineModule.module->setLogger("SpineLogger");
	spineModule.module->setSocketContext(this->inp_context);
	spineModule.module->openSockets();
	if (! spineModule.module->areSocketsValid()) {
		logger->debug("{}: {}", spineModule.moduleName, "Sockets are not valid :(");
		return false;
	}
	// Here we set the module to subscribe to it's name on it's subscriber socket
	// Then configure the module to connect it's publish output to the Spine input
	//  and it's mgmt output too
	spineModule.module->subscribe("Modules");
	spineModule.module->subscribe(spineModule.moduleName);
	spineModule.module->notify(SocketType::PUB, "Spine");
	spineModule.module->notify(SocketType::MGM_OUT, "Spine");
	this->logger->debug(this->nameMsg("Sockets Registered for: " + spineModule.moduleName + "!"));

	this->notify(SocketType::PUB, spineModule.moduleName);
	this->notify(SocketType::MGM_OUT, spineModule.moduleName);
	this->loadedModules.insert(spineModule.moduleName);
	this->logger->info(this->nameMsg("Registered module: " + spineModule.moduleName + "!"));

	auto newThread = thread([&spineModule] () {
		return spineModule.module->run();
	});

	m_threads.push_back(move(newThread));
	m_modules.push_back(spineModule);

	return true;
}

bool Spine::loadModules() {
	return this->loadModules(this->moduleFileLocation);
}

bool Spine::loadModules(const string& directory) {
	// Here we gather a list of relevant module binaries
	//  and then call the load function on each path
	set<string> moduleFiles = this->listModules(directory);

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

bool Spine::isModuleLoaded(std::string moduleName) {
	return this->loadedModules.find(moduleName) != this->loadedModules.end();
}

bool Spine::loadConfig(string location) {
	string confMod = "config";
	if (this->areSocketsValid()) {
		Message wMsg(this->name(), confMod);
		wMsg["data"]["command"] = "load";
		wMsg["data"]["file"] = location;
		return this->sendMessageRecv(SocketType::MGM_OUT, wMsg, [&,confMod](const Message& wMsg) -> bool{
			if (!wMsg["data"].isMember("configLoaded"))
				return false;

			if (wMsg["source"].asString() != confMod || wMsg["destination"].asString() != this->name())
				return false;

			if (!wMsg["data"]["configLoaded"].asBool())
				return false;

			return true;
		});
	} else {
		return false;
	}
}

bool Spine::run() {
	try {
		// Our function here just sleeps for a bit
		//  and then sends a close message to all modules ('Module' channel)
		bool runAgain = true;
		int count = 0;
		while (runAgain) {
			this->pollAndProcess();
			this->logger->debug(this->nameMsg("Sleeping..."));
			this_thread::sleep_for(chrono::milliseconds(500));

			if (count < 120) {
				count++;
			} else {
				runAgain = false;
			}
		}
		// Lets make sure the close command works!
		this->logger->info(this->nameMsg("Closing 'Modules'"));
		Message wMsg(this->name(), "Modules");
		wMsg["data"]["command"] = "close";
		this->sendMessage(SocketType::PUB, wMsg);
		return true;
	} catch(zmq::error_t& ex) {
		this->logger->info(this->nameMsg(ex.what()));
		return false;
	}
}

bool Spine::process_message(const Message& wMsg, CatchState cought, SocketType sockT) {
	this->logger->debug(this->nameMsg(wMsg.asString()));
	return true;
}

}
