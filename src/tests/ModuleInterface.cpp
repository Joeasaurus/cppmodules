#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "ModuleInterfaceFixture.hpp"

SCENARIO("Module private methods work correctly", "[ModuleAPI]") {
	GIVEN("a Module child instance is created") {
		auto module = new InterfaceFixture();

		THEN("module name is 'InterfaceFixture'") {
			REQUIRE(module->name() == "InterfaceFixture");
		}
	}
}
