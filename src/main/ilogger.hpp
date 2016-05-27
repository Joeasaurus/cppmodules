#pragma once

#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"

using namespace std;
using namespace spdlog;

namespace cppm {

	class ILogger {
		public:
			inline ILogger(string name = "Logger");
			virtual ~ILogger(){};

			inline static void setDebug(bool debug = false);
			inline shared_ptr<logger> getLogger() const;

			inline virtual void log(const string& title, const string& data, bool debug = false) const;
			inline virtual void warn(const string& title, const string& data) const;
			inline virtual void err(const string& title, const string& data) const;
			inline virtual void null(const string& data) const;

		private:
			string _name{"Logger"};
			shared_ptr<logger> _logger;

			shared_ptr<sinks::null_sink_mt> _null_sink{make_shared<spdlog::sinks::null_sink_mt> ()};
			shared_ptr<logger> _null_logger;
	};

	ILogger::ILogger(string name) {
		_name = name;

		//queue size must be power of 2
		spdlog::set_async_mode(1048576);

		try {
			_logger = stdout_logger_mt(_name, true);
			_logger->set_pattern("[%T.%e] [%l] %v"); // Custom format

			_null_logger = make_shared<spdlog::logger>("null_logger", _null_sink);
		} catch (const spdlog_ex) {
			_logger = spdlog::get(_name);
			_null_logger = spdlog::get("null_logger");
		}
	};

	void ILogger::setDebug(bool debug) {
		spdlog::set_level(debug ? level::debug : level::notice);
	};

	shared_ptr<logger> ILogger::getLogger() const {
		return _logger;
	};

	void ILogger::log(const string& title, const string& data, bool debug) const {
		if (debug)
			_logger->debug("{}: {}", title, data);
		else
			_logger->info("{}: {}", title, data);
	};

	void ILogger::warn(const string& title, const string& data) const {
		_logger->warn("{}: {}", title, data);
	}

	void ILogger::err(const string& title, const string& data) const {
		_logger->error("{}: {}", title, data);
	};

	void ILogger::null(const string& data) const {
		_null_logger->info(data);
	};

}
