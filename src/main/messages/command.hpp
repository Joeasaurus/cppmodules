#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Command : public Message {
		public:
			Command(string from) : Message(from, CHANNEL::Cmd) {};
	};
}}