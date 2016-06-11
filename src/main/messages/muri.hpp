#pragma once

#include <string>

#include "main/messages/uri.hpp"

using namespace std;

namespace cppm { namespace messages {
	class MUri {
		public:
			MUri(){};
			MUri(const string& inuri) {
				setUri(inuri);
			};
			MUri(const string& mod, const string& dom, const string& com) {
				module(mod);
				domain(dom);
				command(com);
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

			void module(const string& mod) {
				uri.scheme(mod);
			};

			void domain(const string& dom) {
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
