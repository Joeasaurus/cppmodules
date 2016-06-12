#pragma once
#include "main/exceptions/general.hpp"
using namespace std;
using namespace cppm::exceptions::general;

namespace cppm { namespace exceptions { namespace uri {

	class ParamNotFound: public GeneralFailure {
		public:

			ParamNotFound(const string& param) : GeneralFailure( "ParamNotFound" ) {
				message = param;
			};
	};
}}}
