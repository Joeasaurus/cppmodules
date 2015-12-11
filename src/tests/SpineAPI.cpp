#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/spine.hpp"

using namespace cppm;

class SpineTestsFixture {
	private:
		Spine* spine;
	public:
		SpineTestsFixture() {
			spine = new Spine(false);
		}
		~SpineTestsFixture() {
			delete spine;
		}
	protected:
		bool SpineLoadModules() {
			return spine->loadModules(spine->moduleFileLocation);
		}
		bool SpineDefaultModulesLoaded() {
			return spine->isModuleLoaded("config");
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
