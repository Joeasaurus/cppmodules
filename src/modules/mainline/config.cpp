#include "modules/mainline/config.hpp"

ConfigModule::ConfigModule()
{
	this->__info.name   = "Config";
	this->__info.author = "mainline";
}

ConfigModule::~ConfigModule()
{
	this->closeSockets();
	this->logger->debug("{}: {}", this->name(), "Closed");
}

bool ConfigModule::loadConfigFile(std::string filepath)
{
	if (boost::filesystem::is_regular_file(filepath)) {
		try {
			this->config.readFile(filepath.c_str());
			this->logger->debug("{}: {}", this->name(), "Config " + filepath + " loaded!");
			this->configFilepath = filepath;
			return true;
		} catch(const libconfig::FileIOException &fioex) {
			this->logger->debug("{}: {}", this->name(), "Error: I/O error while reading " + filepath);
			return false;
		} catch(const libconfig::ParseException &pex) {
			this->logger->debug() << "{}: {}" << this->name() << "Error: Parse failed at " <<
				pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
			return false;
		}
	} else {
		return false;
	}
}

bool ConfigModule::run()
{
	bool runAgain = true;
	auto reloadTimer = std::chrono::system_clock::now();
	auto reloadTimerFinish = reloadTimer + std::chrono::seconds(120);

	while (runAgain) {
		runAgain = this->pollAndProcess();
		if (runAgain) {
			reloadTimer = std::chrono::system_clock::now();
			if (reloadTimer >= reloadTimerFinish) {
				runAgain = this->loadConfigFile(this->configFilepath);
				reloadTimerFinish = reloadTimer + std::chrono::seconds(120);
			}
		}
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

ConfigModule* loadModule(){return new ConfigModule;}
void unloadModule(ConfigModule* module) {delete module;}