#include "main/module.hpp"
#include "main/spine.hpp"

#include "lib/spdlog/spdlog.h"

int main(int argc, char **argv) {
    spdlog::set_async_mode(1048576); //queue size must be power of 2
    spdlog::set_level(spdlog::level::info);

	std::shared_ptr<spdlog::logger> logger = spdlog::stdout_logger_mt("Logger");
	logger->info("Main started, Logger up");

	Spine *spine = new Spine;
	spine->setLogger(logger);
	zmq::context_t spineCtx(1);

	try {
		spine->setSocketContext(&spineCtx);
		spine->openSockets();
		spine->subscribe(spine->name());
	} catch (const zmq::error_t &e) {
		logger->info("Exception: {}", e.what());
 		return 1;
	}

	logger->info("Loading spine modules");
	spine->loadModules("./modules");
	bool spineReturn = spine->run();

	delete spine;
	return spineReturn;
}