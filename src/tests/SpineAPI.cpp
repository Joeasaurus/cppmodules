#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/spine.hpp"

class SpineTestsFixture {
	private:
		Spine* spine;
	public:
		SpineTestsFixture() {
			spine = new Spine(false);
			REQUIRE(SpineOpenSockets());
		}
		~SpineTestsFixture() {
			delete spine;
		}
	protected:
		bool SpineOpenSockets() {
			REQUIRE_NOTHROW(spine->openSockets());
			REQUIRE(spine->areSocketsValid());
			return spine->areSocketsOpen();
		}
		bool SpineLoadModules() {
			return spine->loadModules(spine->moduleFileLocation);
		}
		bool SpineDefaultModulesLoaded() {
			return spine->isModuleLoaded("mainline_config");
		}
		bool SpineConfigIsLoaded() {
			return spine->loadConfig(spine->moduleFileLocation + "/main.cfg");
		}
 };

TEST_CASE_METHOD(SpineTestsFixture, "All", "[spine,modules,config]") {
	SECTION("Public spine functions") {
		REQUIRE(SpineLoadModules());
		REQUIRE(SpineDefaultModulesLoaded());
		REQUIRE(SpineConfigIsLoaded());
	}
}
