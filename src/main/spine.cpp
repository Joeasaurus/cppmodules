#include "main/spine.hpp"

namespace cppm {

Spine::Spine() : Module("Spine", "Joe Eaves") {
	//TODO: Is there a lot of stuff that could throw here? It needs looking at
	this->inp_context = make_shared<zmq::context_t>(1);
	this->openSockets("__bind__");
	_running.store(true);
}

Spine::~Spine() {
	string closeMessage;

	// Here the Spine sends a message out on it's publish socket
	// Each message directs a 'close' message to a module registered
	//  in the Spine as 'loaded'
	// It then waits for the module to join. It relies on the module closing
	//  cleanly else it will lock up waiting.
	Command wMsg(name());
	wMsg.payload("close");

	//TODO: NOT THREAD SAFE!?!?!
	lock_guard<mutex> lock(_moduleRegisterMutex);
	_running.store(false);
	sendMessage(wMsg);

	for_each(m_threads.begin(), m_threads.end(), [&](thread& t) {
		if (t.joinable()) {
			t.join();
			_logger.log(name(), "Joined", true);
		};
	});

	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	closeSockets();
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
	// lock_guard<mutex> lock(_moduleRegisterMutex, _moduleUnregisterMutex);
	_loadedModules.insert(modName);
	_logger.log(name(), "Registered module: " + modName + "!");
};

void Spine::unregisterModule(const string& modName) {
	// lock_guard<mutex> lock(_moduleRegisterMutex, _moduleUnregisterMutex);
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
			if (com.init(inp_context, name())) {
				registerModule(com.moduleName);

				_logger.log("Spine", "Sockets Registered for: " + com.moduleName + "!", true);

				try {
					com.module->setup();
					while (_running.load()) {
						if (com.isLoaded()) {
							com.module->pollAndProcess();
							com.module->tick();
						} else {
							break;
						}
					}
				} catch (const std::exception& ex) {
					cout << ex.what() << endl;
				}

				// Here the module is essentially dead, but not the thread
				//TODO: Module reloading code, proper shutdown handlers etc.
				com.deinit();
				unregisterModule(com.moduleName);
			}

			this_thread::sleep_for(chrono::milliseconds(1000));
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
	set<string> moduleFiles = listModules(directory);
	boost::filesystem::path moduleLoc(moduleFileLocation);

	for (auto filename : moduleFiles)
	{
		// We should check error messages here better!
		loadModule(filename);
	}

	return true;
}

bool Spine::isModuleLoaded(std::string moduleName) {
	return this->_loadedModules.find(moduleName) != this->_loadedModules.end();
}

bool Spine::process_command(const Message& msg) {
	if (msg.m_from == "config") {
		_logger.log(name(), msg.payload(), true);
		if (msg.payload() == "updated") {
			
			Command msg(name(), "config");
			msg.payload("get-config module-dir");
			sendMessage(msg);
		}
	}
	return true;
}

bool Spine::process_input(const Message& message) {
	return true;
}

}
