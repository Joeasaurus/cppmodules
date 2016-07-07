#include "main/module.hpp"

using namespace cppevent;

namespace cppm {
    Module::Module(string name, string author, bool withHooks) : _withHooks(withHooks) {
        __info.name   = name;
        __info.author = author;
        timeNow = chrono::system_clock::now();
		_eventer = new Eventer();
		if (_withHooks) serviceHooks_Registration();
    }

    Module::~Module(){
        _socketer->closeSockets();
        delete _socketer;
    }

    void Module::polltick(){
        if(_socketer && _socketer->isConnected()) {
	        try {
	            _socketer->pollAndProcess();
	        } catch (NonExistantHook& e) {
	            if (e.isCritical()) {
	                string warning = "[Critical] ";
	                _logger.err(name(), warning + e.what());
	            }
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
		_socketer->on("process_command", [&](const Message& message) {
			try {
				MUri mu(message.payload());
				router.emit(mu);
			} catch (exception& e) {
				_logger.log(name(), e.what(), true);
				return false;
			}
			return true;
		});
	}

	void Module::serviceHooks_Registration() {
		// Register with the spine
		router.on("loaded-success", MUri("pidel://spine/module/loaded/:result"), [&](MUri& mu, URIRouter* _rout) {
			registered = (_rout->getVar("result") == "true");
		});
		_eventer->on("send-registration", [&](chrono::milliseconds) {
			if (! registered) {
				MUri mu("pidel://spine/module/load/" + name());
				mu.send(_socketer, name());
			}
			// TODO: How to remove events?
		}, chrono::milliseconds(500), EventPriority::HIGH);
	}
}
