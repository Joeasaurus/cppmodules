#include "main/module.hpp"


namespace cppm {
    Module::Module(string name, string author) {
        __info.name   = name;
        __info.author = author;
        timeNow = chrono::system_clock::now();
    }

    Module::~Module(){
        _socketer->closeSockets();
        delete _socketer;
    }

    void Module::polltick(){
        if(_socketer && _socketer->isConnected())
        try {
            _socketer->pollAndProcess();
        } catch (NonExistantHook& e) {
            if (e.isCritical()) {
                string warning = "[Critical] ";
                _logger.err(name(), warning + e.what());
            }
        }
        tick();
    }

    string Module::name() const {
		return this->__info.name;
	}

	void Module::connectToParent(string parent, const Context& ctx) {
		_socketer = new Socketer(ctx);
		_socketer->openSockets(name(), parent);
	}
}
