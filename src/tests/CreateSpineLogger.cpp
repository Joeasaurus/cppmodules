#define CATCH_CONFIG_RUNNER
#include "lib/catch/catch.hpp"

#include "main/spine.hpp"
#include "main/segvhandler.hpp"

static std::string LoggerName = "SpineLogger";

TEST_CASE( "SpineLogger is created", "[logger]" ) {
	REQUIRE(spdlog::get(LoggerName)!=nullptr);
	REQUIRE_THROWS_AS(spdlog::stdout_logger_mt(LoggerName), spdlog::spdlog_ex);
}

int main(int argc, char* const argv[]) {
	// Create the logger we will test later
	auto logger = Spine::createLogger(argc > 1 && strcmp(argv[1], "debug") == 0);
	// run tests
	return Catch::Session().run(argc, argv);
}
