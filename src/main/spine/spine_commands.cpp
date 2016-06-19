#include "main/messages/messages.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"
using namespace cppm::messages;
using namespace cppm::exceptions::spine;
using namespace cppm::exceptions::uri;

#include "main/spine/spine.hpp"

namespace cppm {



void Spine::command_moduleLoaded(MUri& mu, map<string, string>& variables) {
	if (variables.find("name") != variables.end()) {
		auto modName = variables["name"];
		registerModule(modName);
		Message m(name(), modName);
		m.setChannel(CHANNEL::Cmd);
		m.payload(modName + "://module/loaded?success=true");
		_socketer->sendMessage(m);
	}
}

}
