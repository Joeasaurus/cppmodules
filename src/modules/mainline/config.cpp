#include "modules/mainline/config.hpp"

ConfigModule::~ConfigModule()
{
	this->closeSockets();
	this->logger->debug(this->nameMsg("Closed"));
}

bool ConfigModule::loadConfigFile(string filepath)
{
	if (boost::filesystem::is_regular_file(filepath)) {
		try {
			this->configOnDisk.fromFile(filepath);
			this->config = this->configOnDisk;
			this->logger->debug(this->nameMsg(filepath + " loaded!"));
			this->logger->debug(this->nameMsg(this->config.asString()));
			this->configFilepath = filepath;
			return true;
		} catch(const exception &ex) {
			this->logger->debug(this->nameMsg("Error: loading " + filepath ));
			this->logger->debug(this->nameMsg(ex.what()));
			return false;
		}
	}
	return false;
}

bool ConfigModule::run()
{
	this->createEvent("ReloadConfig", chrono::milliseconds(5000),
		[&](chrono::milliseconds delta) {
			if(this->loadConfigFile(this->configFilepath)) {
				Message configUpdate(this->name(), "Modules");
				configUpdate["data"]["command"] = "config-update";
				configUpdate["data"]["config"] = this->config.asJson();
				this->logger->debug(this->nameMsg("Config reloaded"));
				return this->sendMessage(SocketType::PUB, configUpdate);
			}
			return false;
		}
	);
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}

	return false;
}

bool ConfigModule::process_message(const Message& wMsg, CatchState cought, SocketType sockT)
{
	//this->logger->debug("{}: {}", this->name(), wMsg.asString());
	if (cought == CatchState::FOR_ME) {
		if (sockT == SocketType::MGM_IN && wMsg["data"].isMember("command")) {
			if (wMsg["data"]["command"].asString() == "load" && wMsg["data"].isMember("file")) {
				Message reply(this->name(), "Spine");

				reply["data"]["configLoaded"] = false;
				if (this->loadConfigFile(wMsg["data"]["file"].asString())) {
					reply["data"]["configLoaded"] = true;
				}

				this->sendMessage(SocketType::MGM_IN, reply);
			}
		}
	}
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}