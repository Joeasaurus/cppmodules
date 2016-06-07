#include "modules/mainline/config.hpp"

bool ConfigModule::loadConfigFile(string filepath)
{
	if (boost::filesystem::is_regular_file(filepath))
		// try {
		// 	this->configOnDisk.fromFile(filepath);
		// 	this->config = this->configOnDisk;
		// 	_logger.log(name(), filepath + " loaded!", true);
		// 	_logger.log(name(), th->config.asString(), true);
		// 	this->configFilepath = filepath;
			return true;
	// 	} catch(const exception &ex) {
	// 		errLog("Error: loading " + filepath);
	// 		errLog(ex.what());
	// 	}
	// }
	return false;
}

void ConfigModule::setup() {
	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_socketer->on("process_input", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_eventer.on("config-reload", [&](chrono::milliseconds) {
		// Never true because the path is static and it's changed now!
		// Need to think about how we specify stuff like that!
		if(this->loadConfigFile(this->configFilepath)) {

			Command configUpdate(this->name());
			configUpdate.payload("global://config/updated");

			if (_socketer->sendMessage(configUpdate))
				_logger.log(name(), "Config reloaded", true);
		}
	}, chrono::milliseconds(5000), EventPriority::LOW);


	Command moduleRunning(name());
	moduleRunning.payload("spine://module/loaded?name=" + name());
	_socketer->sendMessage(moduleRunning);
}

void ConfigModule::tick() {
	_eventer.emitTimedEvents();
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}
