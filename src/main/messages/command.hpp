#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Command : public Message {
		public:
			Command(const string& from) : Message(from, CHANNEL::Cmd) {};
			Command(const string& from, const string& to) : Message(from, to, CHANNEL::Cmd) {};
	};
}}