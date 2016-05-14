#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/spine.hpp"

static std::string LoggerName = "SpineLogger";

SCENARIO("Spine creates a valid logger named 'SpineLogger'", "[spine,logger]") {
	GIVEN("the logger is created") {
		auto logger = cppm::Spine::createLogger(false);

		WHEN("the logger is retrieved") {
			auto gotLogger = spdlog::get(LoggerName);
			THEN("the logger can be retrieved from the registry") {
				REQUIRE(gotLogger != nullptr);
			}
		}

		WHEN("we try to create the logger again") {
			auto secondLogger = cppm::Spine::createLogger(false);
			THEN("we receive the same logger from the registry") {
				REQUIRE(logger == secondLogger);
			}
		}

		WHEN("we try to create the same logger directly") {
			THEN("an exception is thrown") {
				REQUIRE_THROWS_AS(spdlog::stdout_logger_mt(LoggerName), spdlog::spdlog_ex);
			}
		}
	}
}
