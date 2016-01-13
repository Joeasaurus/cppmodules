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

void ConfigModule::run() {
	// _eventer.on("config-reload", [&]() {
	// 	if(this->loadConfigFile(this->configFilepath)) {
	// 		Message configUpdate(this->name(), "Modules");
	// 		configUpdate["data"]["command"] = "config-update";
	// 		configUpdate["data"]["config"] = this->config.asJson();
	// 		_logger.log(name(), "Config reloaded", true);
	// 		return this->sendMessage(SocketType::PUB, configUpdate);
	// 	}
	// 	return false;
	// }, 5, EventPriority::LOW);

	Message moduleRunning(name(), "Spine");
	moduleRunning["data"]["message"] = "module-loaded";
	bool runAgain = this->sendMessageRecv(SocketType::MGM_OUT, moduleRunning, [&](const Message& wMsg) {
		_logger.log(name(), wMsg.asString(), true);
		return true;
	});

	_logger.log(name(), "RUN IS RUNNING", true);
	while (runAgain) {
		_eventer.tick();
		runAgain = this->pollAndProcess();

	}	
}

bool ConfigModule::process_message(const Message& wMsg, CatchState cought, SocketType sockT)
{
	if (cought == CatchState::FOR_ME) {
		if (sockT == SocketType::MGM_IN) {
			if (wMsg["data"].isMember("command")) {
				if (wMsg["data"]["command"].asString() == "load" && wMsg["data"].isMember("file")) {
					Message reply(this->name(), "Spine");

					reply["data"]["configLoaded"] = false;
					if (this->loadConfigFile(wMsg["data"]["file"].asString())) {
						reply["data"]["configLoaded"] = true;
					}

					this->sendMessage(SocketType::MGM_IN, reply);
				}
			} else if (wMsg["data"].isMember("message")) {
				if (wMsg["data"]["message"].asString() == "module-loaded-ack") {
					_logger.log(name(), "Load acknowledged, closing", true);
					return false;
				}
			}
		}
	}
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}