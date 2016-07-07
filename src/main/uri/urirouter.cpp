#include "main/uri/urirouter.hpp"

#include "Routing/Router.h"
#include "Routing/Exceptions.h"
using namespace Routing;
using namespace cppm::exceptions::uri;

namespace cppm {

namespace messages {

URIRouter::URIRouter() {
	router = new Router();
};

void URIRouter::on(const string& name, const MUri& mu, function<void(MUri&, URIRouter*)> hook) {
	routes[mu.command()] = name;
	hooks[name] = hook;
	router->registerPath(mu.command());
};

void URIRouter::emit(MUri& mu) {
	match = new PathMatch(router->matchPath(mu.command()));
	pathMatched = true;
	if (routes.find(match->pathTemplate()) != routes.end()) {
		auto routeName = routes[match->pathTemplate()];
		auto hook = hooks[routeName];
		hook(mu, this);
	}

	// TODO: Should we throw here?
};


// This will be from the last emit
string URIRouter::path() const {
	if (!pathMatched) throw ParamNotFound("No URI has yet been emitted.");
	return match->path();
};
string URIRouter::pathTemplate() const {
	if (!pathMatched) throw ParamNotFound("No URI has yet been emitted.");
	return match->pathTemplate();
};
string URIRouter::getVar(std::string const &name) const {
	if (!pathMatched) throw ParamNotFound("No URI has yet been emitted.");
	try {
		return match->getVar(name);
	} catch (NoSuchElementException& e) {
		throw ParamNotFound(e.what());
	}
};
string URIRouter::operator[] (std::string const &name) const {
	return getVar(name);
};

}}
