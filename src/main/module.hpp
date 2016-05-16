#pragma once
#include "boost/predef.h"

// We have to do some icky things on Windows!
#if BOOST_OS_WINDOWS
	#if defined(MODULE_EXPORT)
		#define CPPMAPI __declspec(dllexport)
	#else
		#define CPPMAPI __declspec(dllimport)
	#endif
#else
	#define CPPMAPI
#endif

// Common
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>

#include <thread>
#include <chrono>
#include <functional>

#include "main/logger.hpp"
#include "main/messages/messages.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"

using namespace std;
namespace algorithm = boost::algorithm;

namespace cppm {
	using namespace messages;
	class ModuleCOM;

	typedef struct ModuleInfo {
		string name = "undefined module";
		string author = "mainline";
	} ModuleInfo;

	class Module {
		friend class ModuleCOM;
		private:
			chrono::system_clock::time_point timeNow;

		protected:
			Socketer*  _socketer;
			Logger     _logger;
			ModuleInfo __info;

		public:
			Module(string name, string author) {
				this->__info.name   = name;
				this->__info.author = author;
				this->timeNow = chrono::system_clock::now();
			};
			virtual ~Module(){
				_socketer->closeSockets();
				delete _socketer;
			};

			virtual void polltick(){
				if(_socketer && _socketer->isConnected())
				try {
					_socketer->pollAndProcess();
				} catch (NonExistantHook& e) {
					if (e.isCritical()) {
						string warning = "[Critical] ";
						_logger.err(name(), warning + e.what());
					}
				}
				tick();
			};
			virtual void tick(){};
			virtual void setup()=0;

			inline string name() const;
			inline bool connectToParent(string p, shared_ptr<context_t> ctx);
	};

	string Module::name() const {
		return this->__info.name;
	};

	bool Module::connectToParent(string p, shared_ptr<context_t> ctx) {
		if (!_socketer)
			_socketer = new Socketer(ctx);

		if (!_socketer->isConnected())
			_socketer->openSockets(name(), p);

		return _socketer->isConnected();
	};
}

/* Export the module
 *
 * These functions should be overriden and set 'export "C"' on.
 * These functions allow us to load the module dynamically via <dlfcn.h>
 */
typedef CPPMAPI cppm::Module* Module_ctor(void);
typedef CPPMAPI void Module_dctor(cppm::Module*);
