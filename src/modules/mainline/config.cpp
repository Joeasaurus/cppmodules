#include "modules/mainline/config.hpp"

ConfigModule::~ConfigModule()
{
	this->closeSockets();
	_logger.log(name(), "Closed", true);
}

bool ConfigModule::loadConfigFile(string filepath)
{
	if (boost::filesystem::is_regular_file(filepath))
		// try {
		// 	this->configOnDisk.fromFile(filepath);
		// 	this->config = this->configOnDisk;
		// 	_logger.log(name(), filepath + " loaded!", true);
		// 	_logger.log(name(), this->config.asString(), true);
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
	_eventer.on("config-reload", [&](chrono::milliseconds delta) {

		if(this->loadConfigFile(this->configFilepath)) {

			Command configUpdate(this->name());
			configUpdate.payload("Config Updated");
			
			if (sendMessage(configUpdate))
				_logger.log(name(), "Config reloaded", true);

			return true;
		}

		return false;

	}, chrono::milliseconds(5000), EventPriority::LOW);


	Command moduleRunning(name());
	moduleRunning.payload("module-loaded");
	sendMessage(moduleRunning);
}

void ConfigModule::tick() {
	_eventer.emitTimedEvents();
}

bool ConfigModule::process_command(const Message& message)
{
	_logger.log(name(), message.format(), true);
	return true;
}

bool ConfigModule::process_input(const Message& message)
{
	_logger.log(name(), message.format(), true);
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}