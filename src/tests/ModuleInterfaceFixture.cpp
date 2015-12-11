#include "ModuleInterfaceFixture.hpp"
InterfaceFixture::~InterfaceFixture() {
	this->closeSockets();
	this->logger->debug(this->nameMsg("Closed"));
};

bool InterfaceFixture::run() {
	return false;
};

bool InterfaceFixture::process_message(const Message& message, CatchState cought, SocketType sockT) {
	return true;
};