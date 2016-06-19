// $mapping = $global_mapping;
// $uri = array("xyz", ":apple", "apply");
// for($part = 0; $part < count($uri); $part++) {
//   $cur_part = $uri[$part];
//   $mapping = &$maping[$next_part];
// }
// $mapping == the final route unless we threw an error above;

#include <map>
#include <string>
#include "main/global.hpp"
#include "main/uri/muri.hpp"

#include "Routing/Router.h"
#include "Routing/PathMatch.h"
#include "Routing/Exceptions.h"
using namespace Routing;

using namespace std;

namespace cppm { namespace messages {

class URIRouter {
private:
	Router router;
	map<string, string> routes; // URI, name
	map<string, function<void(MUri& mu, PathMatch&)>> hooks; // name, hook
public:
	void on(const string& name, const MUri& mu, function<void(MUri&, PathMatch&)> hook) {
		routes[mu.command()] = name;
		hooks[name] = hook;
		router.registerPath(mu.command());
	}

	void emit(MUri& mu) {
		auto match = router.matchPath(mu.command());
		if (routes.find(match.pathTemplate()) != routes.end()) {
			auto routeName = routes[match.pathTemplate()];
			auto hook = hooks[routeName];
			hook(mu, match);
		}

		// TODO: Should we throw here?
	}
};

}}
