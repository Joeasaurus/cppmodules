

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>


#include "main/messages/messages.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"
// Module Specific

using namespace cppm::messages;
using namespace cppm::exceptions::spine;

#include "main/spine.hpp"
#include "main/modulecom.hpp"

namespace cppm {

Spine::Spine() : Module("Spine", "Joe Eaves") {
	//TODO: Is there a lot of stuff that could throw here? It needs looking at
	if (connectToParent("__bind__", Context::getSingleContext())) {;

		// Hack to die after so long while we're in dev.
		_eventer.on("close-timeout", [&](chrono::milliseconds) {
			_running.store(false);
		}, chrono::milliseconds(60000), EventPriority::HIGH);

		hookSocketCommands();

		auto index = chainFactory.create();
		chainFactory.insert(index, "input");
		chainFactory.insert(index, "input");
		assert(index == 1);

		list<unsigned long> refList{index};
		authoredChains["output"] = refList;

		_running.store(true);
	}
}

Spine::~Spine() {
	_running.store(false);

	// Here the Spine sends a message out on it's publish socket
	// Each message directs a 'close' message to a module registered
	//  in the Spine as 'loaded'
	// It then waits for the module to join. It relies on the module closing
	//  cleanly else it will lock up waiting.
	Command wMsg(name());
	wMsg.payload("close");

	_socketer->sendMessage(wMsg);

	for_each(m_threads.begin(), m_threads.end(), [&](thread& t) {
		if (t.joinable()) {
			t.join();
			_logger.log(name(), "Joined", true);
		};
	});

	// After all modules are closed we don't need our sockets
	//  so we should close them before exit, to be nice
	_logger.log(name(), "Closed");
}

void Spine::tick(){
	_eventer.emitTimedEvents();
};

set<string> Spine::listModuleFiles(const string& directory) const {
	set<string> moduleFiles;

	listModuleFiles(moduleFiles, directory);

	return moduleFiles;
}

void Spine::listModuleFiles(set<string>& destination, const string& directory) const {
	// Here we list a directory and build a vector of files that match
	//  our platform specific dynamic lib extension (e.g., .so)
	try {
		boost::filesystem::recursive_directory_iterator startd(directory), endd;
		auto files = boost::make_iterator_range(startd, endd);

		for(boost::filesystem::path p : files){
			if (p.extension() == this->moduleFileExtension &&
				!boost::filesystem::is_directory(p)
			) {
				destination.insert(p.string());
			}
		}
	} catch(boost::filesystem::filesystem_error& e) {
		_logger.warn(name(), "WARNING! Could not load modules!");
		throw InvalidModulePath(directory, static_cast<string>(e.what()));
	}

}

void Spine::registerModule(const string& modName) {
	lock_guard<mutex> lock(_moduleRegisterMutex);
	_loadedModules.insert(modName);
	_logger.log(name(), "Registered module: " + modName + "!");
};

void Spine::unregisterModule(const string& modName) {
	lock_guard<mutex> lock(_moduleRegisterMutex);
	_loadedModules.erase(modName);
	_logger.log(name(), "Unregistered module: " + modName + "!");
};

bool Spine::loadModule(const string& filename) {
	// Strart a new thread holding a modulecom.
	// The com will load the module from disk and then re-init it while it's loaded.
	// We can add logic for unloading it later
	// For now we've added a check on the atomic _running bool, so the Spine can tell us to unload,
	//   so as to close the module and make the thread join()able.
	m_threads.push_back(thread([&,filename] {
		ModuleCOM com(filename);
		_logger.log(name(), "Loading " + boost::filesystem::basename(filename) + "...", true);

		if (com.loadLibrary()) {
			if (com.initModule(name(), _socketer->getContext()) && _running.load()) {

				registerModule(com.moduleName);

				_logger.log("Spine", "Sockets Registered for: " + com.moduleName + "!", true);

				try {
					com.module->setup();
					while (_running.load()) {
						if (com.isLoaded()) {
							try {
								com.module->polltick();
							} catch (exception& e) {
								cout << "CAUGHT POLLTICK IN SPINE " << e.what() << endl;
							}
						} else {
							break;
						}
					}
				} catch (const std::exception& ex) {
					cout << ex.what() << endl;
				}

				// Here the module is essentially dead, but not the thread
				//TODO: Module reloading code, proper shutdown handlers etc.
				com.deinitModule();
				unregisterModule(com.moduleName);
			}

			this_thread::sleep_for(chrono::milliseconds(1000));
			com.unloadLibrary();
		}

		// If we get here the thread is joinable so it's ready for reaping!
	}));

	return true;
}

bool Spine::loadModules(const string& directory) {
	// Here we gather a list of relevant module binaries
	//  and then call the load function on each path
	set<string> moduleFiles;
	try {
		listModuleFiles(moduleFiles, directory);
	} catch (InvalidModulePath& e) {
		_logger.log(name(), e.what(), true);
		return false;
	}

	_logger.log(name(), directory, true);

	for (auto filename : moduleFiles)
	{
		//TODO: We should check error messages here better!
		loadModule(filename);
	}

	return true;
}

bool Spine::isModuleLoaded(std::string moduleName) {
	return this->_loadedModules.find(moduleName) != this->_loadedModules.end();
}

set<string> Spine::loadedModules() {
	return _loadedModules;
}

void Spine::hookSocketCommands() {
	_socketer->on("process_command", [&](const Message& msg) {
		_logger.log(name(), "CMD HEARD " + msg.payload(), true);
		if (msg.m_from == "config") {
			if (msg.payload() == "updated") {

				Command msg(name(), "config");
				msg.payload("get-config module-dir");
				_socketer->sendMessage(msg);
			}
		}
		return true;
	});

	_socketer->on("process_input", [&](const Message& msg) {
		_logger.log(name(), "INPUT HEARD " + msg.payload(), true);
		return true;
	});

	_socketer->on("process_output", [&](const Message& msg) {
		// HERE ENSUES THE ROUTING
		// The spine manages chains of modules, so we forward from out to in down the chains
		_logger.log(name(), "OUTPUT HEARD " + msg.serialise(), true);

		auto modChain = msg.getChain();

		// Channel has already told us it's output to be routed
		// Output is never directed, input is!
		// If chainID == 0, get list of author chains for message author/sender
		// Create refs for each of those chains
		// Set the chain on the msg and send to the current() on it
		// If the chain next() is null, kill it

		Input inmsg(msg.m_from);
		inmsg.payload(msg.payload());

		if (modChain.first == 0) {
			_logger.log(name(), "Creating chains for " + msg.m_from, true);

			auto chains = authoredChains[msg.m_from];

			for (auto& chain : chains) {
				auto ref = chainFactory.create(chain);
				// _logger.log(name(), "... created " + to_string(chain) + "," + to_string(ref) + " ... with current() => " + chainFactory.current(chain, ref), true);

				inmsg.setChain(chain, ref);
				inmsg.sendTo(chainFactory.current(chain, ref));

				chainFactory.next(chain, ref);
				chainFactory.hasEnded(chain, ref, true); // kill it!

				_socketer->sendMessage(inmsg);
			}
		} else if (chainFactory.has(modChain.first, modChain.second)) {
			inmsg.sendTo(chainFactory.current(modChain.first, modChain.second));

			chainFactory.next(modChain.first, modChain.second);
			chainFactory.hasEnded(modChain.first, modChain.second, true); // kill it!

			_socketer->sendMessage(inmsg);
		} else {
			_logger.log(name(), "DEAD message: " + msg.serialise());
			return false;
		}

		return true;
	});
}

bool Spine::isRunning() {
	return _running.load();
}

}

cppm::Spine* createModule(){return new cppm::Spine;}
void destroyModule(cppm::Spine* module) {delete module;}
