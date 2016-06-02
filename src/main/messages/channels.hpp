#pragma once

#include <string>
#include <map>

namespace cppm {
	enum CHANNEL {
		None,
		In,
		Out,
		Cmd,
		DELIM

	};

	static std::map<std::string, CHANNEL> strToChan {
		{"0", CHANNEL::None   },
		{"1", CHANNEL::In     },
		{"2", CHANNEL::Out    },
		{"3", CHANNEL::Cmd    },
		{"4", CHANNEL::DELIM  }
	};

	static std::map<CHANNEL, std::string> chanToStr {
		{CHANNEL::None,  "0"},
		{CHANNEL::In,    "1"},
		{CHANNEL::Out,   "2"},
		{CHANNEL::Cmd,   "3"},
		{CHANNEL::DELIM, "4"}
	};
}
