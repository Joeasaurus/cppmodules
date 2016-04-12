#pragma once

#include <string>

using namespace std;

namespace cppm {
	static map<string, string> Channels = {
		{ "none",    "NONE"    },
		{ "command", "COMMAND" },
		{ "in",      "INPUT"   },
		{ "out",     "OUTPUT"  }
	};

	namespace messages {
		static vector<string> tokeniseString(const string& message, const string& spchar) {
			vector<string> messageTokens;
			if (!message.empty()) {
				boost::split(messageTokens, message, boost::is_any_of(spchar));
			}

			return messageTokens;
		};
		class Message {
			protected:
				string data    = Channels["none"];
			public:
				string _to      = Channels["none"];
				string _from    = Channels["none"];
				Message(){};
				Message(string from) {
					_from = from;
				};
				Message(string from, string to) : Message(from) {
					_to = to;
				};
				string payload() const {
					return data;
				};
				bool payload(string in, bool data_only = true) {
					if (data_only) {
						data = in;
						return true;
					}

					auto tokens = tokeniseString(in, " ");
					if (tokens.size() < 3) 
						return false;

					//TODO: This is SLOW
					_to   = tokens.at(0); tokens.erase(tokens.begin());
					_from = tokens.at(0); tokens.erase(tokens.begin());
					data  = tokens.at(0); tokens.erase(tokens.begin());
					for (auto& tk : tokens) 
						data += " " + tk;

					return true;
				}
				string format() const {
					return _to + " " + _from + " " + data;
				}
		};
	}
}