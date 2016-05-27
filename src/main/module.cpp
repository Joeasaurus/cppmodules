#include "main/module.hpp"


namespace cppm {
    string Module::name() const {
		return this->__info.name;
	}

	bool Module::connectToParent(string p, const Context& ctx) {
		if (!_socketer)
			_socketer = new Socketer(ctx);

		if (!_socketer->isConnected())
			_socketer->openSockets(name(), p);

		return _socketer->isConnected();
	}
}
