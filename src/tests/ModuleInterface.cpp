#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "ModuleInterfaceFixture.hpp"

SCENARIO("A module performs public API correctly", "[spine,logger]") {
	GIVEN("the logger is created") {
		auto module = new InterfaceFixture();

		WHEN("The module is created") {
			THEN("run() returns false!") {
				REQUIRE(!module->run());
			}
		}

		WHEN("The module is created") {
			const Message msg;
			THEN("process_message() returns true") {
				REQUIRE(module->process_message(msg, CatchState::FOR_ME, SocketType::PUB));
			}
		}
	}
}