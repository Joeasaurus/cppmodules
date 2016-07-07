#pragma once
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include "boost/predef.h"
#include "main/exceptions/exceptions.hpp"

using namespace std;

namespace cppm {

static vector<string> tokeniseString(const string& message, const string& spchar) {
	vector<string> messageTokens;
	if (!message.empty()) {
		boost::split(messageTokens, message, boost::is_any_of(spchar));
	}

	return messageTokens;
};

}

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
