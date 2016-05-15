#pragma once
// Common
#include "main/module.hpp"
#include "main/logger.hpp"
// Module Specific
#include <dlfcn.h>
#include <mutex>
#include <string>
#include <set>

#include <boost/filesystem.hpp>

namespace cppm {
	class ModuleCOM {
		private:
			const string name = "ModuleCOM";
			string _filename;
			Logger _logger;

			void* _module_so = nullptr;
			Module_ctor* createModule = nullptr;
			Module_dctor* destroyModule = nullptr;

			bool _moduleLoaded = false;
			bool _moduleInit   = false;

			inline bool openLibraryFile();
			inline int  loadLibrarySymbols();

		public:
			Module* module = nullptr;
			string moduleName;

			inline ModuleCOM(const string& filename) {_filename = filename;};
			inline bool loadLibrary();
			inline void unloadLibrary();

			inline bool initModule(const string& parent, shared_ptr<context_t> ctx);
			inline void deinitModule();

			inline bool isLoaded() const;
	};

	// const string ModuleCOM::name = "ModuleCOM";

	bool ModuleCOM::openLibraryFile() {
		// Here we use dlopen to load the dynamic library (that is, a compiled module)
		_module_so = dlopen(_filename.c_str(), RTLD_NOW | RTLD_GLOBAL);
		if (!_module_so) {
			_logger.err(name, dlerror());
			return false;
		}
		return true;
	};

	int ModuleCOM::loadLibrarySymbols() {
		// Here we use dlsym to hook into exported functions of the module
		// One to load the module class and one to unload it
		createModule = (Module_ctor*)dlsym(_module_so, "createModule");
		if (!createModule) {
			_logger.err(name, dlerror());
			return 1;
		}
		_logger.log(name, "Resolved loadModule", true);

		destroyModule = (Module_dctor*)dlsym(_module_so, "destroyModule");
		if (!destroyModule) {
			return 2;
		}
		_logger.log(name, "Resolved unloadModule", true);

		return 0;
	}

	bool ModuleCOM::loadLibrary() {
		// In the threads we load the binary and hook into it's exported functions.
		// We use the load function to create an instance of it's Module-derived class
		// We then set up an interface with the module using our input/output sockets
		//  which are a management Rep and Req socket for commands in and out
		//  and a chain-building pair of Pub and Sub sockets for message passing.
		// Now we run it's run() function in a new thread and push that on to our list.
		if (! openLibraryFile()) {
			_logger.err(name, "Failiure " + boost::filesystem::basename(_filename) + "..");
			return false;
		}
		_logger.log(name, "Opened file " + boost::filesystem::basename(_filename) + "..", true);

		if (loadLibrarySymbols() != 0) {
			_logger.err(name, "Failiure " + boost::filesystem::basename(_filename) + "..");
			return false;
		}
		_logger.log(name, "Functions resolved", true);

		_moduleLoaded = true;
		return _moduleLoaded;
	};

	void ModuleCOM::unloadLibrary() {
		if (_moduleInit) {
			deinitModule();
		}

		if (dlclose(_module_so) != 0) {
			_logger.err(name, "Could not dlclose module file");
		}
	};

	bool ModuleCOM::initModule(const string& parent, shared_ptr<context_t> ctx) {
		if (!_moduleInit) {
			module = createModule();
			if (module) {
				moduleName = module->name();
				module->connectToParent(parent, ctx);
				_moduleInit = true;
			}
		}

		return _moduleInit;
	};

	void ModuleCOM::deinitModule() {
		if (_moduleInit && module) {
			delete module;
			module = nullptr;
		}
	};

	bool ModuleCOM::isLoaded() const {
		return _moduleLoaded;
	}
}
