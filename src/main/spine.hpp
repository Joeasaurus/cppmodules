#pragma once
// Common
#include "main/logger.hpp"
#include "main/messages/messages.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"
// Module Specific
#include <dlfcn.h>
#include <mutex>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <set>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "main/module.hpp"
#include "main/modulecom.hpp"

#include "Eventer.hpp"

using namespace zmq;
using namespace cppm::messages;
using namespace cppm::exceptions::spine;
using namespace cppevent;

namespace cppm {
	class Spine : public Module {
		private:
			#if BOOST_OS_MACOS
				string moduleFileExtension = ".dylib";
			#else
				#if BOOST_OS_WINDOWS
					string moduleFileExtension = ".dll";
				#else // Linux
					string moduleFileExtension = ".so";
				#endif
			#endif

			atomic<bool> _running{false};
			vector<thread> m_threads;
			mutex _moduleRegisterMutex; // Protects loadedModules
			set<string> _loadedModules;
			Eventer _eventer;

			set<string> listModuleFiles(const string& directory) const;
			void listModuleFiles(set<string>& destination, const string& directory) const;
			bool isModuleFile(const string& filename);

			void registerModule(const string& name);
			void unregisterModule(const string& name);

			void hookSocketCommands();

		public:
			Spine(shared_ptr<context_t> ctx);
			~Spine();
			void setup(){};
			void tick();

			bool process_command(const Message& msg);
			bool process_input  (const Message& msg);
			bool process_output (const Message& msg);

			bool loadModules(const string& directory);
			bool loadModule(const string& filename);

			set<string> loadedModules();

			bool isModuleLoaded(std::string moduleName);
			bool isRunning();

	};
}

/*
 * The spine shall be a HUB for all messages between modules.
 * Modules are black-box I/O units. Messages go in and messages come out
 * A Module (the spine included) shall have two sockets: PUB & SUB
 * Each module shall be connected in a spine-centric star topology
 * There shall be three channels for messages between a module and the spine:
 *   - 'COMMAND' This is for control messages between the two, e.g. config changes
 *   - 'INPUT' This is data from spine to be processed by module
 *   - 'OUTPUT' This is data from module to be handled by spine
 * The spine shall be responsible for managing logical chains of modules
 * The spine will route OUTPUT messages from moduleA to INPUT of moduleB
 */
