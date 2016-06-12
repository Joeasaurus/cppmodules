#pragma once
#include "main/exceptions/general.hpp"
using namespace std;
using namespace cppm::exceptions::general;

namespace cppm { namespace exceptions { namespace spine {

	class InvalidModulePath: public GeneralFailure {
		public:
			InvalidModulePath(const string& dir, const string& msg) : GeneralFailure("InvalidModulePath") {
				path = dir;
				message = msg;
			};
			
		private:
			string path;
	};
}}}
