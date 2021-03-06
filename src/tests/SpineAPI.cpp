#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/spine.hpp"

using namespace cppm;

SCENARIO("Spine basic initialisation", "[SpineAPI]") {
	GIVEN("a spine is created") {
		auto context = make_shared<zmq::context_t>(1);
		Spine spine(context);
		REQUIRE(spine.isRunning());

		WHEN("nothing else has happened") {
			THEN("the spine has no loaded modules") {
				REQUIRE(spine.loadedModules().size() == 0);
			}
		}
	}
}
