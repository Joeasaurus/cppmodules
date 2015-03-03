#ifndef H_MODULE
#define H_MODULE
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <zmq.hpp>

typedef struct ModuleInfo {
	std::string name;
	std::string author;
} ModuleInfo;

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
		virtual void close()=0;
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
			this->inp_in->close();
			this->inp_out->close();
			this->inp_manage_in->close();
			this->inp_manage_out->close();
		};
		void connectOutput(std::string socketType, std::string endpoint) {
			endpoint = "inproc://" + endpoint;
			if (socketType == "publish") {
				endpoint += ".in";
				this->inp_out->connect(endpoint.c_str());
			} else if (socketType == "manage") {
				endpoint += ".manage";
				this->inp_manage_out->connect(endpoint.c_str());
			}

		};
		void subscribe(std::string channel) {
			this->inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.c_str(), 4);
		};
		void sendMessage(std::string messageType, std::string message, std::string module) {
			if (messageType == "publish") {
				message = module + " " + message;
				try {
					zmq::message_t zmqObject(message.size());
					memcpy(zmqObject.data(), message.data(), message.size());
					int failure = this->inp_out->send(zmqObject);
					std::cout << "Sent " << message << std::endl;
				} catch (const zmq::error_t &e) {
					std::cout << e.what() << std::endl;
				}
			} else if (messageType == "manage-in") {
				try {
					zmq::message_t zmqObject(message.size());
					memcpy(zmqObject.data(), message.data(), message.size());
					int failure = this->inp_manage_in->send(zmqObject);
					std::cout << "Sent " << message << std::endl;
				} catch (const zmq::error_t &e) {
					std::cout << e.what() << std::endl;
				}
			} else if (messageType == "manage-out") {
				try {
					zmq::message_t zmqObject(message.size());
					memcpy(zmqObject.data(), message.data(), message.size());
					int failure = this->inp_manage_out->send(zmqObject);
					std::cout << "Sent " << message << std::endl;
				} catch (const zmq::error_t &e) {
					std::cout << e.what() << std::endl;
				}
			}
		};
		std::string recvMessage(std::string socketType) {
			std::string errorText = "__NULL_RECV_FAILED__";
			zmq::message_t message;
			std::string messageText;
			if (socketType == "publish") {
		        if(this->inp_in->recv(&message)) {
		        	messageText = std::string(static_cast<char*>(message.data()), message.size());
       			} else {
       				messageText = errorText;
       			}
			} else if (socketType == "manage-in") {
				if(this->inp_manage_in->recv(&message)) {
		        	messageText = std::string(static_cast<char*>(message.data()), message.size());
       			} else {
       				messageText = errorText;
       			}
			} else if (socketType == "manage-out") {
				if(this->inp_manage_out->recv(&message)) {
		        	messageText = std::string(static_cast<char*>(message.data()), message.size());
       			} else {
       				messageText = errorText;
       			}
			}
			return messageText;
		};

		std::vector<std::string> &splitString(const std::string &s, char delim, std::vector<std::string> &elems) {
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				if (!item.empty()) {
					elems.push_back(item);
				 }
			}
			return elems;
		};
		std::vector<std::string> splitString(const std::string &s, char delim) {
			std::vector<std::string> elems;
			splitString(s, delim, elems);
			return elems;
		};
};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);

#endif // H_MODULE