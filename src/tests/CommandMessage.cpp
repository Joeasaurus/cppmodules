#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "main/messages/command.hpp"

using namespace cppm::messages;

SCENARIO("Command messages have special properties", "[Messages, Command]") {
	GIVEN("we create a Command message") {
		Command cmd("Test");
		string commandData = "Spine:chains:add:1:input";

		WHEN("we add some command values") {
			cmd.module("Spine");
			cmd.domain("chains");
			cmd.command("add:1:input");

			THEN("the Command payload is " + commandData) {
				REQUIRE(cmd.payload() == commandData);
			}
		}

		WHEN("we set the payload to " + commandData) {
			cmd.payload(commandData);

			THEN("the Command's module is set to Spine") {
				REQUIRE(cmd.module() == "Spine");
			}

			THEN("the Command's domain is set to chains") {
				REQUIRE(cmd.domain() == "chains");
			}

			THEN("the Command's command is add:1:input") {
				REQUIRE(cmd.command() == "add:1:input");
			}
		}

	}
}
