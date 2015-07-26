#define CATCH_CONFIG_RUNNER
#include "lib/catch/catch.hpp"

#include "main/spine.hpp"
#include "main/segvhandler.hpp"

static std::string LoggerName = "SpineLogger";

class SpineTestsFixture {
	private:
		Spine spine;
	protected:
		bool CreateSpine() {
			return SpineOpenSockets();
		}
		bool SpineOpenSockets() {
			REQUIRE_NOTHROW(spine.openSockets());
			REQUIRE(spine.areSocketsValid());
			return spine.areSocketsOpen();
		}
		bool SpineDefaultModulesLoaded() {
			spine.loadModules(spine.moduleFileLocation);
			REQUIRE(spine.isModuleLoaded("mainline_config"));
			return true;
		}
 };

TEST_CASE_METHOD(SpineTestsFixture, "Spine functions", "[spine]") {
	SECTION("Spine is created") {
		REQUIRE(CreateSpine());
	}
	SECTION("Spone loads default modules") {
		REQUIRE(CreateSpine());
		REQUIRE(SpineDefaultModulesLoaded());
	}
}

TEST_CASE( "SpineLogger is created", "[logger]" ) {
	REQUIRE(spdlog::get(LoggerName) != nullptr);
	REQUIRE_THROWS_AS(spdlog::stdout_logger_mt(LoggerName), spdlog::spdlog_ex);
}

int main(int argc, char* const argv[]) {
	// Create the logger we will test later
	auto logger = Spine::createLogger(argc > 1 && strcmp(argv[1], "debug") == 0);

	// run tests
	return Catch::Session().run(argc, argv);
}
