#include "main/interfaces/logger.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"

using namespace std;
using namespace spdlog;

namespace cppm {

namespace interfaces {

    struct LoggerImpl {
        string _name{"Logger"};
        shared_ptr<logger> _logger;

        shared_ptr<sinks::null_sink_mt> _null_sink{make_shared<spdlog::sinks::null_sink_mt> ()};
        shared_ptr<logger> _null_logger;
    };

	Logger::Logger(string name) {
        _handle = new LoggerImpl();
        _handle->_name = name;

		//queue size must be power of 2
		spdlog::set_async_mode(1048576);

		try {
			_handle->_logger = stdout_logger_mt(name, true);
			_handle->_logger->set_pattern("[%T.%e] [%l] %v"); // Custom format

			_handle->_null_logger = make_shared<spdlog::logger>("null_logger", _handle->_null_sink);
		} catch (const spdlog_ex) {
			_handle->_logger = spdlog::get(name);
			_handle->_null_logger = spdlog::get("null_logger");
		}
	};

	void Logger::setDebug(bool debug) {
		spdlog::set_level(debug ? level::debug : level::notice);
	};

	logger* Logger::getLogger() const {
		return _handle->_logger.get();
	};

	void Logger::log(const string& title, const string& data, bool debug) const {
		if (debug)
			_handle->_logger->debug("{}: {}", title, data);
		else
			_handle->_logger->info("{}: {}", title, data);
	};

	void Logger::warn(const string& title, const string& data) const {
		_handle->_logger->warn("{}: {}", title, data);
	}

	void Logger::err(const string& title, const string& data) const {
		_handle->_logger->error("{}: {}", title, data);
	};

	void Logger::null(const string& data) const {
		_handle->_null_logger->info(data);
	};

}
}
