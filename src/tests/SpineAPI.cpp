#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "main/spine.hpp"

static std::string LoggerName = "SpineLogger";

class SpineTestsFixture {
	private:
		Spine spine;
	public:
		SpineTestsFixture() {
			REQUIRE(SpineOpenSockets());
		}
	protected:
		bool SpineOpenSockets() {
			REQUIRE_NOTHROW(spine.openSockets());
			REQUIRE(spine.areSocketsValid());
			return spine.areSocketsOpen();
		}
		bool SpineLoadModules() {
			return spine.loadModules(spine.moduleFileLocation);
		}
		bool SpineDefaultModulesLoaded() {
			return spine.isModuleLoaded("mainline_config");
		}
 };

TEST_CASE_METHOD(SpineTestsFixture, "All", "[spine]") {
	SECTION("Spine loads default modules") {
		REQUIRE(SpineLoadModules());
		REQUIRE(SpineDefaultModulesLoaded());
	}
}

int main(int argc, char* const argv[]) {
	// Create the logger we will test later
	auto logger = Spine::createLogger(false);
	// run tests
	return Catch::Session().run(argc, argv);
}
