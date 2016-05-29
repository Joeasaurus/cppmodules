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

#include <chrono>
#include <functional>

using namespace std;

namespace cppm {

class Context;
class Module;
class ModuleCOM;

typedef struct ModuleInfo {
    string name = "undefined module";
    string author = "mainline";
} ModuleInfo;

    namespace interfaces {
    	class IModule {
    		friend class ModuleCOM;

    		public:
    			static IModule* create(string name, string author);
    			virtual ~IModule(){};

    			virtual void polltick()=0;
    			virtual void tick()=0;
    			virtual void setup()=0;

    			virtual string name() const = 0;
    			virtual bool connectToParent(string p, const Context& ctx)=0;
    	};
    }

}
/* Export the module
 *
 * These functions should be overriden and set 'export "C"' on.
 * These functions allow us to load the module dynamically via <dlfcn.h>
 */
typedef CPPMAPI cppm::interfaces::IModule* Module_ctor(void);
typedef CPPMAPI void Module_dctor(cppm::interfaces::IModule*);
