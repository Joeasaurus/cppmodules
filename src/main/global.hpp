#pragma once
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace cppm {

namespace messages {
static vector<string> tokeniseString(const string& message, const string& spchar) {
	vector<string> messageTokens;
	if (!message.empty()) {
		boost::split(messageTokens, message, boost::is_any_of(spchar));
	}

	return messageTokens;
};
}

}
