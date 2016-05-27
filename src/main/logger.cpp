#include "main/logger.hpp"
#include "main/ilogger.hpp"

namespace cppm {
Logger::Logger(string name) {
    m_logger = new ILogger(name);
}

Logger::~Logger() {
    delete m_logger;
}

void Logger::setDebug(bool debug) {
    m_logger->setDebug(debug);
}

void Logger::log(const string& title, const string& data, bool debug) const {
    m_logger->log(title, data, debug);
}
void Logger::warn(const string& title, const string& data) const {
    m_logger->warn(title, data);
}
void Logger::err(const string& title, const string& data) const {
    m_logger->err(title, data);
}
void Logger::null(const string& data) const {
    m_logger->null(data);
}

}
