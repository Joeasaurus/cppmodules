#pragma once

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
using namespace std;

namespace cppm { namespace exceptions { namespace socketer {

	class NonExistantHook: public runtime_error {
		public:

			NonExistantHook(const string& hook) : runtime_error( "NonExistantHook" ), hookName(hook) {};

			virtual const char* what() const {
				ostringstream cnvt;
				cnvt.str("");

				cnvt << runtime_error::what() << ": " << hookName;

				return strdup(cnvt.str().c_str());
			}

			bool isCritical() const {
				if (hookName == "process_command" || hookName == "process_input")
					return true;
				return false;
			}
		private:
			const string hookName;
	};
}}}
