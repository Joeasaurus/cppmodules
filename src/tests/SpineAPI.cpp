#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/spine.hpp"

using namespace cppm;

SCENARIO("Spine basic initialisation", "[Spine, SpineAPI]") {
	GIVEN("a spine is created") {
		Spine spine;
		REQUIRE(spine.isRunning());

		WHEN("nothing else has happened") {
			THEN("the spine has no loaded modules") {
				REQUIRE(spine.loadedModules().size() == 0);
			}
		}
	}
}
