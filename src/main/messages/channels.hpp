#pragma once

#include <string>
#include <map>

namespace cppm {
	enum CHANNEL {
		None,
		In,
		Out,
		Cmd

	};
	
	static map<string, CHANNEL> strToChan {
		{"0", CHANNEL::None   },
		{"1", CHANNEL::In     },
		{"2", CHANNEL::Out    },
		{"3", CHANNEL::Cmd}
	};

	static map<CHANNEL, string> chanToStr {
		{CHANNEL::None,    "0"},
		{CHANNEL::In,      "1"},
		{CHANNEL::Out,     "2"},
		{CHANNEL::Cmd, "3"}
	};
}