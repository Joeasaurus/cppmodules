#include "modules/mainline/config.hpp"

ConfigModule::~ConfigModule()
{
	this->closeSockets();
	_logger.log(name(), "Closed", true);
}

bool ConfigModule::loadConfigFile(string filepath)
{
	if (boost::filesystem::is_regular_file(filepath)) {
		try {
			this->configOnDisk.fromFile(filepath);
			this->config = this->configOnDisk;
			_logger.log(name(), filepath + " loaded!", true);
			_logger.log(name(), this->config.asString(), true);
			this->configFilepath = filepath;
			return true;
		} catch(const exception &ex) {
			errLog("Error: loading " + filepath);
			errLog(ex.what());
		}
	}
	return false;
}

void ConfigModule::setup() {
	_eventer.on("config-reload", [&](chrono::milliseconds delta) {
		if(this->loadConfigFile(this->configFilepath)) {
			// Message configUpdate(this->name(), Channels["command"]);
			// configUpdate["data"]["command"] = "config-update";
			// configUpdate["data"]["config"] = this->config.asJson();
			// _logger.log(name(), "Config reloaded", true);
			return true;// this->sendMessage(configUpdate);
		}
		_logger.log(name(), "CONFIG RELOAD", true);
	}, chrono::milliseconds(5000), EventPriority::LOW);


	Message moduleRunning(name(), Channels["command"]);
	moduleRunning["data"]["message"] = "module-loaded";
	sendMessage(moduleRunning);
}

void ConfigModule::tick() {
	_eventer.emitTimedEvents();
}

bool ConfigModule::process_message(const Message& wMsg)
{
	_logger.log(name(), wMsg.asString(), true);
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}