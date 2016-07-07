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

namespace Routing {
	class Router;
	class PathMatch;
};

using namespace std;
using namespace Routing;

namespace cppm { namespace messages {

class URIRouter {
	private:
		bool pathMatched = false;
		Router* router;
		PathMatch* match;
		map<string, string> routes; // URI, name
		map<string, function<void(MUri& mu, URIRouter*)>> hooks; // name, hook
	public:
		URIRouter();
		void on(const string& name, const MUri& mu, function<void(MUri&, URIRouter*)> hook);
		void emit(MUri& mu);

		virtual string path() const;
		virtual string pathTemplate() const;
		virtual string getVar(std::string const &name) const;
		virtual string operator[] (std::string const &name) const;
};

}}
