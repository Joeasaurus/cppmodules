#pragma once

#include <string>
#include "main/messages/command.hpp"
#include "main/messages/input.hpp"
#include "main/messages/output.hpp"

using namespace std;

namespace cppm {
	

	namespace messages {
		class Factory {
			public:
				string identify(const Message& message) {
					
					if (tokens.at(0) == Channels["none"]) {
						return Channels["none"];
					}
				}
		};
	}
}