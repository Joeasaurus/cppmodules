#define BOOST_TEST_MODULE ModuleTests
#include <boost/test/included/unit_test.hpp>

#include "main/module.hpp"
#include "tests/support/testmodule.hpp"

BOOST_AUTO_TEST_CASE(module_create)
{
	TestModule mod;
	BOOST_REQUIRE(mod.name() == "TestModule");
}
