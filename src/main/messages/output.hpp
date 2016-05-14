#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Output : public Message {
		public:
			Output(string from) : Message(from, CHANNEL::Out) {};
	};
}}