#pragma once

#include <string>
#include "main/messages/message.hpp"

using namespace std;

namespace cppm { namespace messages {
	class Command : public Message {
		public:
			Command(const string& from) : Message(from, CHANNEL::Cmd) {};
			Command(const string& from, const string& to) : Message(from, to, CHANNEL::Cmd) {};

			string module() {
				return _module;
			};

			string domain() {
				return _domain;
			};

			string command() {
				return _command;
			};

			void module(string mod) {
				_module = mod;
			};

			void domain(string dom) {
				_domain = dom;
			};

			void command(string com) {
				_command = com;
			};

			string payload() const {
				if (_data.empty())
					return _module + ":" + _domain + ":" + _command;
				else
					return _data;
			};

			bool payload(string in) {
				auto parts = tokeniseString(in, ":");
				module(parts.at(0));
				if (parts.size() >= 2) {
					domain(parts.at(1));
					if (parts.size() >= 3) {
						command(parts.at(2));
						parts.erase(parts.begin());
						parts.erase(parts.begin());
						parts.erase(parts.begin());
						for (auto& tk : parts)
							_command += ":" + tk;
					}
				}
				_data = in;
				return true;
			};

		private:
			string _module  = "";
			string _domain  = "";
			string _command = "";
	};
}}
