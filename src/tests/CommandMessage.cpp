#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "main/messages/command.hpp"

using namespace cppm::messages;

static const string commandData = "spine://chains/add?id=1&module=input";

SCENARIO("Command messages are URIs with named fields", "[Messages, Command]") {
	GIVEN("we create a Command message") {
		Command cmd("Test");


		WHEN("we add some command values") {
			cmd.module("spine");
			cmd.domain("chains");
			cmd.command("add");
			cmd.param(make_pair("id", "1"));
			cmd.param(make_pair("module", "input"));

			THEN("the Command payload is " + commandData) {
				REQUIRE(cmd.payload() == commandData);
			}
		}

		WHEN("we set the payload to " + commandData) {
			cmd.payload(commandData);

			THEN("the Command's module is set to spine") {
				REQUIRE(cmd.module() == "spine");
			}

			THEN("the Command's domain is set to chains") {
				REQUIRE(cmd.domain() == "chains");
			}

			THEN("the Command's command is add") {
				REQUIRE(cmd.command() == "add");
			}
		}

	}
}

SCENARIO("Command messages can be created from parent class (Message)", "[Messages, Command]") {
	GIVEN("we create a Message with a URI payload") {
		Message m("Test");
		m.payload(commandData);

		REQUIRE(m.payload() == commandData);

		WHEN("we create a Command message by copy constructor") {
			Command com(m);

			THEN("the payload hasn't changed") {
				REQUIRE(com.payload() == commandData);
			}

			THEN("the Command's module is set to spine") {
				REQUIRE(com.module() == "spine");
			}

			THEN("the Command's domain is set to chains") {
				REQUIRE(com.domain() == "chains");
			}

			THEN("the Command's command is add") {
				REQUIRE(com.command() == "add");
			}
		}
	}
}
