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
				this->config = json::to_object(json::parse(configFile));
				this->logger->debug(this->nameMsg(filepath + " loaded!"));
				this->logger->debug(this->nameMsg(json::stringify(this->config)));
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
	this->createEvent("ReloadConfig", chrono::milliseconds(15000),
		[&](chrono::milliseconds delta) {
			return this->loadConfigFile(this->configFilepath);
		}
	);
	bool runAgain = true;
	while (runAgain) {
		runAgain = this->pollAndProcess();
	}

	return false;
}

bool ConfigModule::process_message(const json::value& message,
								   CatchState cought, SocketType sockT)
{
	this->logger->debug("{}: {}", this->name(), json::stringify(message));
	if (cought == CatchState::FOR_ME) {
		if (sockT == SocketType::MGM_IN && json::has_key(message["data"], "command")) {
			if (json::to_string(message["data"]["command"]) == "load" &&
				json::has_key(message["data"], "file")
			) {
				json::object msg{
					{ "configLoaded", "false" }
				};
				if (this->loadConfigFile(json::to_string(message["data"]["file"]))) {
					msg["configLoaded"] = "true";
				}
				this->sendMessage(SocketType::MGM_IN, "Spine", msg);
			}
		}
	}
	return true;
}

ConfigModule* createModule(){return new ConfigModule;}
void destroyModule(ConfigModule* module) {delete module;}