#pragma once

#include <string>

#include "main/uri/uri.hpp"
#include "main/messages/channels.hpp"
#include "main/messages/socketer.hpp"

using namespace std;

namespace cppm { namespace messages {
	class MUri {
		public:
			MUri(){};
			MUri(const string& inuri) {
				setUri(inuri);
			};
			MUri(const string& mod, const string& dom, const string& com) {
				scheme(mod);
				cdomain(dom);
				command(com);
			};

			void send(Socketer* sock, const string& from, const string& to = "", CHANNEL chan = CHANNEL::Cmd) const {
				Message msg(from, to);
				msg.setChannel(chan);
				msg.payload(getUri());
				sock->sendMessage(msg);
			};

			const string scheme() const {
				return uri.scheme();
			};

			const string cdomain() const {
				return uri.host();
			};

			const string command() const {
				return uri.path();
			};

			const map<string, list<string>>& params() const {
				return uri.getQueryParams();
			};

			void scheme(const string& mod) {
				uri.scheme(mod);
			};

			void cdomain(const string& dom) {
				uri.host(dom);
			};

			void command(string com) {
				if (strncmp(com.c_str(), "/", 1))
					com = "/" + com;
				uri.path(com);
			};

			const list<string>& param(const string& key) const {
				return uri.getQueryParam(key);
			}

			void param(const string& key, const string& value) {
				uri.addQueryParam(key, value);
			};

			string getUri() const {
				return uri.toString();
			};

			void setUri(const string& inuri) {
				uri.parseUri(inuri);
			};

		private:
			Uri uri;
	};
}}
