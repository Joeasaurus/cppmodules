#pragma once

#include <string>

using namespace std;

namespace cppm {

class ILogger;

class Logger {
    public:
        Logger(string name = "Logger");
        ~Logger();

        void setDebug(bool debug = false);

        void log(const string& title, const string& data, bool debug = false) const;
        void warn(const string& title, const string& data) const;
        void err(const string& title, const string& data) const;
        void null(const string& data) const;

    private:
        ILogger* m_logger;
};
}
