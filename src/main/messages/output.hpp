#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Ouput : public Message {
		public:
			Ouput(string from) : Message(from, CHANNEL::Out) {};
	};
}}