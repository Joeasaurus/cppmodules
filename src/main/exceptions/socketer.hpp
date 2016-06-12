#pragma once
#include "main/exceptions/general.hpp"
using namespace std;
using namespace cppm::exceptions::general;

namespace cppm { namespace exceptions { namespace socketer {

	class NonExistantHook: public GeneralFailure {
		public:
			NonExistantHook(const string& hook) : GeneralFailure("NonExistantHook") {
				message = hook;
			};

			bool isCritical() const CPPM_CLANGNOEXCEPT {
				if (message == "process_command" || message == "process_input")
					return true;
				return false;
			}
	};
}}}
