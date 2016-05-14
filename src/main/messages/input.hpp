#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Input : public Message {
		public:
			Input(string from) : Message(from, CHANNEL::In) {};
	};
}}