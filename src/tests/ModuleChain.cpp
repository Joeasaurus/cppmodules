#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/chain.hpp"

using namespace cppm;

SCENARIO("Module chains can hold modules", "[Spine, ModuleChains]") {
	GIVEN("we create a chain") {

		Chain chain1;
		REQUIRE(chain1.length() == 0);

		WHEN("we add a module") {

			chain1.insert("module-one");

			THEN("the chain length increases by one") {
				REQUIRE(chain1.length() == 1);
			}

			WHEN("we add another module") {

				chain1.insert("module-two");

				THEN("the length increases by one") {
					REQUIRE(chain1.length() == 2);
				}

				THEN("the chain's 'current' reference doesn't change") {
					REQUIRE(chain1.current() == "module-one");
				}
			}
		}
	}
}

SCENARIO("Module chains reference count", "[Spine, ModuleChains]") {
	GIVEN("we create a chain with two modules") {

		Chain chain;
		chain.insert("module-one");
		chain.insert("module-two");
		REQUIRE(chain.length() == 2);

		THEN("the current reference is the first module") {
			REQUIRE(chain.current() == "module-one");
		}

        WHEN("we call next() on the chain") {
			REQUIRE(chain.next() == "module-two");

			THEN("the current reference moves") {
				REQUIRE(chain.current() == "module-two");
			}

			WHEN("we create a copy of the chain") {
				THEN("the new chain's reference is reset") {
					REQUIRE(chain.copy()->current() == "module-one");
				}
			}

			WHEN("we call next() again") {
				THEN("the chain throws 50") {
					REQUIRE_THROWS_AS(chain.next(), int);
				}
			}
		}
	}
}
