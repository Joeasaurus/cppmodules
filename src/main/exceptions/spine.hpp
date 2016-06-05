#pragma once

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
using namespace std;

namespace cppm { namespace exceptions { namespace spine {

	class InvalidModulePath: public runtime_error {
		public:

			InvalidModulePath(const string& dir, const string& msg)
				: runtime_error( "InvalidModulePath" ), path(dir), message(msg) {};

			virtual const char* what() const {
				ostringstream cnvt;
				cnvt.str("");

				cnvt << runtime_error::what() << ": " << message;

				return strdup(cnvt.str().c_str());
			}
		private:
			string path;
			string message;
	};
}}}
