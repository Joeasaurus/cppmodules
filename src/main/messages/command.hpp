#pragma once

#include <string>

#include "main/messages/message.hpp"
#include "main/messages/uri.hpp"

using namespace std;

namespace cppm { namespace messages 	
	class Command : public Message {
		public:
			Command(const string& from) : Message(from) {m_chan = CHANNEL::Cmd;};
			Command(const string& from, const string& to) : Command(from) {m_to = to;};
			Command(const Message& rhs) : Command(rhs.m_from, rhs.m_to) {
				m_chantype = rhs.m_chantype;
				_chainID   = rhs._chainID;
				_chainRef  = rhs._chainRef;
				_data = rhs._data;
				payReload();
			};

			const string module() {
				return uri.scheme();
			};

			const string domain() {
				return uri.host();
			};

			const string command() {
				auto c = uri.path();
				if (strcmp(&c[0], "/"))
					return c.substr(1);
				return c;
			};

			const std::vector<std::pair<string, string>> params() {
				return uri.getQueryParams();
			};

			void module(string mod) {
				uri.scheme(mod);
			};

			void domain(string dom) {
				uri.host(dom);
			};

			void command(string com) {
				if (strncmp(com.c_str(), "/", 1))
					com = "/" + com;
				uri.path(com);
			};

			void param(const pair<string, string>& paramPair) {
				uri.addQueryParam(paramPair);
			};

			string payload() const {
				return uri.toString();
			};

			string payReload() {
				payload(_data);
				return uri.toString();
			};

			bool payload(string in) {
				uri.parseUri(in);
				_data = in;
				m_chan = CHANNEL::Cmd;
				return true;
			};

		private:
			Uri uri;
	};
}}
