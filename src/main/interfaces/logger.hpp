#pragma once

#include <string>
using namespace std;

namespace spdlog {
	class logger;
}
namespace cppm {

namespace interfaces {

	struct LoggerImpl;

	class Logger {
		public:
			Logger(string name = "Logger");
			virtual ~Logger(){};

			static void setDebug(bool debug = false);
			spdlog::logger* getLogger() const;

			virtual void log(const string& title, const string& data, bool debug = false) const;
			virtual void warn(const string& title, const string& data) const;
			virtual void err(const string& title, const string& data) const;
			virtual void null(const string& data) const;

		private:
			LoggerImpl* _handle;
	};

}
}
