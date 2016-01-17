#include "main/spine.hpp"

namespace cppm {

Spine::Spine() : Module("Spine", "Joe Eaves") {
	//TODO: Is there a lot of stuff that could throw here? It needs looking at
	this->inp_context = make_shared<zmq::context_t>(1);
	this->openSockets();
	_running.store(true);
}

Spine::~Spine() {
	string closeMessage;

	// Here the Spine sends a message out on it's publish socket
	// Each message directs a 'close' message to a module registered
	//  in the Spine as 'loaded'
	// It then waits for the module to join. It relies on the module closing
	//  cleanly else it will lock up waiting.
	Message wMsg(this->name(), "");
	wMsg["data"]["command"] = "close";

	//TODO: NOT THREAD SAFE!?!?!
	lock_guard<mutex> lock(_moduleRegisterMutex);
	_running.store(false);
	for_each(_loadedModules.begin(), _loadedModules.end(), [&](string module) {
		wMsg["destination"] = module;
		this->sendMessage(SocketType::PUB, wMsg);
	});

	for_each(m_threads.begin(), m_threads.end(), [&](thread& t) {
		if (t.joinable()) {
			t.join();
			_logger.log(name(), "Joined", true);
		};
	});

	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	this->closeSockets();
	_logger.log(name(), "Closed");
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
		_logger.getLogger()->warn(name() + ": WARNING! Could not load modules!");
		_logger.getLogger()->warn(name() + ":     - " + static_cast<string>(e.what()));
	}

	return moduleFiles;
}

void Spine::registerModule(const string& modName) {
	lock_guard<mutex> lock(_moduleRegisterMutex, _moduleUnregisterMutex);
	_loadedModules.insert(modName);
	notify(SocketType::PUB, modName);
	notify(SocketType::MGM_OUT, modName);
	_logger.log(name(), "Registered module: " + modName + "!");
};

void Spine::unregisterModule(const string& modName) {
	lock_guard<mutex> lock(_moduleRegisterMutex, _moduleUnregisterMutex);
	_loadedModules.erase(modName);
	_logger.log(name(), "Unregistered module: " + modName + "!");
};

bool Spine::loadModule(const string& filename) {
	// Strart a new thread holding a modulecom.
	// The com will load the module from disk and then re-init it while it's loaded.
	// We can add logic for unloading it later
	// For now we've added a check on the atomic _running bool, so the Spine can tell us to unload,
	//   so as to close the module and make the thread joinable().
	m_threads.push_back(thread([&,filename] {
		ModuleCOM com(filename);
		_logger.log(name(), "Loading " + boost::filesystem::basename(filename) + "...", true);

		if (com.load()) {
			while (com.isLoaded() && _running.load()) {
				if (com.init(inp_context, name())) {
					registerModule(com.moduleName);

					_logger.log("Spine", "Sockets Registered for: " + com.moduleName + "!", true);

					if (com.module->areSocketsValid()) {
						_logger.log("Spine", "Sockets valid", true);
					}

					try {
						//_logger.log(mod->name(), "NAME", true);
						com.module->run();
						// Here the thread is essentially dead, but not the module
						//TODO: Module reloading code, proper shutdown handlers etc.

					} catch (const std::exception& ex) {
						cout << ex.what() << endl;
					}

					com.deinit();
					unregisterModule(com.moduleName);
				}
				this_thread::sleep_for(chrono::milliseconds(1000));
			}

			com.unload();
		}

		// If we get here the thread is joinable so it's ready for reaping!
	}));

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
	return this->_loadedModules.find(moduleName) != this->_loadedModules.end();
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

void Spine::run() {
	try {
		// Our function here just sleeps for a bit
		//  and then sends a close message to all modules ('Module' channel)
		bool runAgain = true;
		int count = 0;

		while (runAgain) {
			this->pollAndProcess();
			_logger.log(name(), "Sleeping...", true);
			this_thread::sleep_for(chrono::milliseconds(500));

			if (count < 120) {
				runAgain = pollAndProcess();
				count++;

			} else {
				runAgain = false;
			}
		}

		// Lets make sure the close command works!
		_logger.log(name(), "Closing 'Modules'");
		Message wMsg(this->name(), "Modules");
		wMsg["data"]["command"] = "close";
		this->sendMessage(SocketType::PUB, wMsg);

	} catch(zmq::error_t& ex) {
		errLog(ex.what());
	}
}

bool Spine::process_message(const Message& wMsg, CatchState cought, SocketType sockT) {
	_logger.log(name(), wMsg.asString(), true);

	if (cought == CatchState::FOR_ME) {
		if (sockT == SocketType::MGM_IN && wMsg["data"].isMember("message")) {
			if (wMsg["data"]["message"].asString() == "module-loaded") {
				this->_loadedModules.insert(wMsg["source"].asString());

				Message reply(this->name(), wMsg["source"].asString());
				reply["data"]["message"] = "module-loaded-ack";
				this->sendMessage(SocketType::MGM_IN, reply);
			}
		}
	}
	return true;
}

}
