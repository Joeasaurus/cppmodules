#pragma once
#include "boost/predef.h"

// We have to do some icky things on Windows!
#if BOOST_OS_WINDOWS
	#if defined(MODULE_EXPORT)
		#define CPPM_WINEXPORT __declspec(dllexport)
	#else
		#define CPPM_WINEXPORT __declspec(dllimport)
	#endif
#else
	#define CPPM_WINEXPORT
#endif

#include <string>
#include <chrono>
#include <functional>
#include "main/interfaces/logger.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"
#include "main/messages/messages.hpp"

using namespace std;

namespace cppm {

	typedef struct ModuleInfo {
	    string name = "undefined module";
	    string author = "mainline";
	} ModuleInfo;

	class ModuleCOM;

	using namespace messages;

	class Module {
		friend class ModuleCOM;
		private:
			chrono::system_clock::time_point timeNow;

		protected:
			Socketer*  _socketer = nullptr;
			Logger     _logger;
			ModuleInfo __info;

			bool registered = false;

		public:
			Module(string name, string author);
			virtual ~Module();

			virtual void polltick();
			virtual void tick(){};
			virtual void setup()=0;

			string name() const;
			void connectToParent(string parent, const Context& ctx);
	};
}

/* Export the module
 *
 * These functions should be overriden and set 'export "C"' on.
 * These functions allow us to load the module dynamically via <dlfcn.h>
 */
typedef CPPM_WINEXPORT cppm::Module* Module_ctor(void);
typedef CPPM_WINEXPORT void Module_dctor(cppm::Module*);
