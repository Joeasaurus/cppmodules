#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "main/messages/muri.hpp"

using namespace cppm::messages;

static const string commandData = "spine://chains/add?id=1&module=input";

SCENARIO("MUri's are URIs with named fields", "[Messages, MUri]") {
	GIVEN("we create an MUri") {
		MUri cmd;


		WHEN("we add some command values") {
			cmd.module("spine");
			cmd.domain("chains");
			cmd.command("add");
			cmd.param(make_pair("id", "1"));
			cmd.param(make_pair("module", "input"));

			THEN("the uri payload is " + commandData) {
				REQUIRE(cmd.getUri() == commandData);
			}
		}

		WHEN("we set the uri to " + commandData) {
			cmd.setUri(commandData);

			THEN("the module field is set to spine") {
				REQUIRE(cmd.module() == "spine");
			}

			THEN("the domain field is set to chains") {
				REQUIRE(cmd.domain() == "chains");
			}

			THEN("the command field is add") {
				REQUIRE(cmd.command() == "add");
			}
		}

	}
}
