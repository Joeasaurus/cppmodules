#pragma once

#include <string>

#include "spdlog/spdlog.h"

using namespace std;
using namespace spdlog;

namespace cppm {

	class Logger {
		public:
			inline Logger(string name = "Logger");

			inline void setDebug(bool debug = false);
			inline shared_ptr<logger> getLogger() const;

			inline virtual void log(const string& title, const string& data, bool debug = false);

		private:
			string _name;
			shared_ptr<logger> _logger;

	};

	Logger::Logger(string name) {
		_name = name;

		//queue size must be power of 2
		spdlog::set_async_mode(1048576);

		try {
			_logger = stdout_logger_mt(_name);
		} catch (const spdlog_ex) {
			_logger = spdlog::get(_name);
		}

		_logger->set_pattern("[%T.%e] [%l] %v"); // Custom format

		log("Logger", "Logger Open!", true);
	};


	void Logger::setDebug(bool debug) {
		spdlog::set_level(debug ? level::debug : level::info);
	};

	shared_ptr<logger> Logger::getLogger() const {
		return _logger;
	};

	void Logger::log(const string& title, const string& data, bool debug) {
		if (debug)
			_logger->debug("{}: {}", title, data);
		else
			_logger->info("{}: {}", title, data);
	};

}


	
