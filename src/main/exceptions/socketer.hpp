#pragma once
#include "boost/predef.h"

// VS2-013 doesn't have noexcept!
#if BOOST_OS_WINDOWS
	#define CPPM_CLANGNOEXCEPT
#else
	#define CPPM_CLANGNOEXCEPT noexcept
#endif

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
using namespace std;

namespace cppm { namespace exceptions { namespace socketer {

	class NonExistantHook: public runtime_error {
		public:

			NonExistantHook(const string& hook) : runtime_error( "NonExistantHook" ), hookName(hook) {};

			virtual const char* what() const CPPM_CLANGNOEXCEPT {
				ostringstream cnvt;
				cnvt.str("");

				cnvt << runtime_error::what() << ": " << hookName;

				return strdup(cnvt.str().c_str());
			}

			bool isCritical() const CPPM_CLANGNOEXCEPT {
				if (hookName == "process_command" || hookName == "process_input")
					return true;
				return false;
			}
		private:
			const string hookName;
	};
}}}
