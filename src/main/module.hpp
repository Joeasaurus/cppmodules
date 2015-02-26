#include <string>
#include <iostream>

#ifndef H_MODULE
#define H_MODULE
#include <zmq.hpp>

typedef struct ModuleInfo {
	std::string name;
	std::string author;
} ModuleInfo;

class Module {
	protected:
		ModuleInfo __info;
		zmq::context_t* ipc_context;
	    zmq::socket_t* ipc_manage;
	public:
		virtual ~Module(){};
		virtual void close()=0;
		virtual std::string name()=0;
		void setSocketContext(zmq::context_t* context) {
			ipc_context = context;
		};
		void openSockets(std::string moduleName) {
			std::string pubipc = "ipc://" + moduleName;
			ipc_manage = new zmq::socket_t(*ipc_context, ZMQ_REP);
			ipc_manage->bind(pubipc.c_str());
		};
		void closeSockets() {
			this->ipc_manage->close();
		};
};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);

#endif // H_MODULE