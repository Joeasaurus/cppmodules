#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "main/chain.hpp"
#include "main/chainfactory.hpp"

using namespace cppm;

SCENARIO("Module chains can hold modules", "[Spine, ModuleChains]") {
	GIVEN("we create a chain") {

		Chain chain1;

		THEN("the length is 0") {
			REQUIRE(chain1.length() == 0);
		}

		WHEN("we add a module") {

			chain1.insert("module-one");

			THEN("the chain length increases by one") {
				REQUIRE(chain1.length() == 1);
			}

			AND_WHEN("we add another module") {

				chain1.insert("module-two");

				THEN("the length increases by one") {
					REQUIRE(chain1.length() == 2);
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

		THEN("the current reference is the first module") {
			REQUIRE(chain.current() == "module-one");
		}

        AND_WHEN("we call next() on the chain") {
			chain.next();

			THEN("the current reference moves") {
				REQUIRE(chain.current() == "module-two");
			}

			AND_WHEN("we create a copy of the chain") {
				THEN("the new chain's reference is reset") {
					REQUIRE(chain.copy()->current() == "module-one");
				}
			}

			AND_WHEN("we call next() again, we reach the end") {
				REQUIRE(chain.next() == chain.nullchar);
				REQUIRE(chain.ended());

				THEN("current() still gives us the last module") {
					REQUIRE(chain.current() == "module-two");
				}
			}
		}
	}
}

SCENARIO("Chain Factory can create and hold chains", "[Spine, ModuleChains]") {
	GIVEN("we create a factory") {
		ChainFactory cf;

		WHEN("we create the first new chain") {
			unsigned long chainID = cf.create();

			THEN("the id is 1") {
				REQUIRE(chainID == 1);
			}

			WHEN("we create a second new chain") {
				unsigned long secondChainID = cf.create();

				THEN("the id is 2") {
					REQUIRE(secondChainID == 2);
				}
			}

			GIVEN("we add a module to the chain") {
				cf.insert(chainID, "module");

				WHEN("we create an instance of that chain") {
					unsigned long chainRef = cf.create(chainID);

					THEN("the instance exists") {
						REQUIRE(cf.has(chainID, chainRef));
					}

					THEN("due to single length, we reach the end") {
						REQUIRE(cf.hasEnded(chainID, chainRef));
					}
				}
			}
		}
	}
}

SCENARIO("Chain factory can walk and delete chains") {
	GIVEN("We create a factory with a two-module chain") {
		ChainFactory cf;
		auto idx = cf.create();
		cf.insert(idx, "module-one");
		cf.insert(idx, "module-two");

		WHEN("we create an instance of the chain") {
			auto ref = cf.create(idx);

			AND_WHEN("we call next on the chain") {
				cf.next(idx, ref);

				THEN("the chain has ended") {
					REQUIRE(cf.current(idx, ref) == "module-two");
					REQUIRE(cf.hasEnded(idx, ref));
				}

				AND_THEN("we can delete the chain because it hasEnded") {
					REQUIRE(cf.hasEnded(idx, ref, true));
					REQUIRE_FALSE(cf.has(idx, ref));
				}
			}
		}

	}
}
