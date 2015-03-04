#ifndef H_MODULE
#define H_MODULE
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <zmq.hpp>

#include <boost/algorithm/string.hpp>

typedef struct ModuleInfo {
	std::string name;
	std::string author;
} ModuleInfo;

enum class SocketType {
	PUB,
	SUB,
	MGM_IN,
	MGM_OUT
};

class Module {
	protected:
		ModuleInfo __info;
		zmq::context_t* inp_context;
		zmq::socket_t* inp_manage_in;
		zmq::socket_t* inp_manage_out;
		zmq::socket_t* inp_in;
		zmq::socket_t* inp_out;
	public:
		virtual ~Module(){};
		virtual std::string name()=0;
		virtual void run()=0;
		void setSocketContext(zmq::context_t* context) {
			this->inp_context = context;
		};
		void openSockets() {
			std::string inPoint = "inproc://" + this->name() + ".in";
			std::string outPoint = "inproc://" + this->name() + ".out";
			std::string managePoint = "inproc://" + this->name() + ".manage";
			try {
				this->inp_in = new zmq::socket_t(*this->inp_context, ZMQ_SUB);
				this->inp_out = new zmq::socket_t(*this->inp_context, ZMQ_PUB);
				this->inp_manage_in = new zmq::socket_t(*this->inp_context, ZMQ_REP);
				this->inp_manage_out = new zmq::socket_t(*this->inp_context, ZMQ_REQ);

				this->inp_in->bind(inPoint.c_str());
				this->inp_manage_in->bind(managePoint.c_str());
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		void closeSockets() {
			try {
				this->inp_in->close();
				this->inp_out->close();
				this->inp_manage_in->close();
				this->inp_manage_out->close();
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}

			delete this->inp_in;
			delete this->inp_out;
			delete this->inp_manage_in;
			delete this->inp_manage_out;
		};
		void connectOutput(SocketType sockT, std::string endpoint) {
			endpoint = "inproc://" + endpoint;
			try {
				if (sockT == SocketType::PUB) {
					endpoint += ".in";
					this->inp_out->connect(endpoint.c_str());
				} else if (sockT == SocketType::MGM_OUT) {
					endpoint += ".manage";
					this->inp_manage_out->connect(endpoint.c_str());
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		void subscribe(std::string channel) {
			try{
				this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}
		};
		bool sendMessage(SocketType sockT, std::string message) {
			bool sendErr = false;

			zmq::message_t zmqObject(message.size());
			memcpy(zmqObject.data(), message.data(), message.size());

			try {
				if (sockT == SocketType::PUB) {
					sendErr = this->inp_out->send(zmqObject);
				} else if (sockT == SocketType::MGM_IN) {
					sendErr = this->inp_manage_in->send(zmqObject);
				} else if (sockT == SocketType::MGM_OUT) {
					sendErr = this->inp_manage_out->send(zmqObject);
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}

			return sendErr;
		};
		std::string recvMessage(SocketType sockT) {
			std::string errorText = "__NULL_RECV_FAILED__";
			std::string messageText;

			zmq::message_t message;
			bool recvErr = false;

			try {
				if (sockT == SocketType::PUB) {
					recvErr = this->inp_in->recv(&message);
				} else if (sockT == SocketType::MGM_IN) {
					recvErr = this->inp_manage_in->recv(&message);
				} else if (sockT == SocketType::MGM_OUT) {
					recvErr = this->inp_manage_out->recv(&message);
				}
			} catch (const zmq::error_t &e) {
				std::cout << e.what() << std::endl;
			}

			if(recvErr) {
				messageText = std::string(static_cast<char*>(message.data()), message.size());
			} else {
				messageText = errorText;
			}

			return messageText;
		};
};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);

#endif // H_MODULE