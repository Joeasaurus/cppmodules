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
			ifstream configFile(filepath);
			if (configFile) {
				this->reader.parse(configFile, this->config, false);
				this->logger->debug(this->nameMsg(filepath + " loaded!"));
				this->logger->debug(this->nameMsg(this->config.asString()));
				this->configFilepath = filepath;
				return true;
			}
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
	this->logger->debug(this->nameMsg("Run function executed!"));
	this->createEvent("ReloadConfig", chrono::milliseconds(1000),
		[&](chrono::milliseconds delta) {
			if(this->loadConfigFile(this->configFilepath)) {
				WireMessage configUpdate(this->name(), "Module");
				configUpdate.message["data"]["command"] = "config-update";
				configUpdate.message["data"]["config"] = this->config;
				return this->sendMessage(SocketType::PUB, configUpdate);
			}
			return false;
		}
	);
	this->logger->debug(this->nameMsg("ReloadConfig event created"));
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}

	return false;
}

bool ConfigModule::process_message(const WireMessage& wMsg, CatchState cought, SocketType sockT)
{
	this->logger->debug("{}: {}", this->name(), wMsg.message.asString());
	if (cought == CatchState::FOR_ME) {
		if (sockT == SocketType::MGM_IN && wMsg.message["data"].isMember("command")) {
			if (wMsg.message["data"]["command"].asString() == "load" && wMsg.message["data"].isMember("file")) {
				WireMessage reply(this->name(), "Spine");
				
				reply.message["data"]["configLoaded"] = false;
				if (this->loadConfigFile(wMsg.message["data"]["file"].asString())) {
					reply.message["data"]["configLoaded"] = true;
				}
				
				this->sendMessage(SocketType::MGM_IN, reply);
			}
		}
	}
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}