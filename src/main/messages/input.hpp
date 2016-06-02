#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Input : public Message {
		public:
			Input(const string& from) : Message(from, CHANNEL::In) {};
			Input(const string& from, const string& to) : Message(from, to, CHANNEL::In) {};
	};
}}
