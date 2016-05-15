#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "main/logger.hpp"

using namespace cppm;

static std::string LoggerName = "Logger";

SCENARIO("cppm::Logger works and is thread-safe", "[general,logger]") {
	GIVEN("A cppm::Logger instance is created") {
		Logger logger;

		THEN("the spdlog::logger can be retrieved from the registry") {
			REQUIRE(spdlog::get(LoggerName) != nullptr);
		}

		WHEN("we create another cppm::Logger instance") {
			Logger logger2;
			THEN("we receive the same spdlog::logger from the registry") {
				REQUIRE(logger.getLogger() == logger2.getLogger());
			}
		}

		WHEN("we try to create a new spdlog::logger directly, with the same name") {
			THEN("an exception is thrown") {
				REQUIRE_THROWS_AS(spdlog::stdout_logger_mt(LoggerName), spdlog::spdlog_ex);
			}
		}
	}
}
