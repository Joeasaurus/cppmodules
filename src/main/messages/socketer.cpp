#include "main/messages/socketer.hpp"
namespace cppm { namespace messages {

Socketer::~Socketer() {
    closeSockets();
};

const Context& Socketer::getContext() {
    return inp_context;
};

void Socketer::openSockets(string name, string parent) {
    this->name = name;
    lock_guard<mutex> lock(_moduleSockMutex);
    if (!_connected) {
        string inPoint = "inproc://";
        string outPoint = "inproc://";

        try {
            inp_in = new socket_t(*inp_context.getContext(), ZMQ_SUB);
            inp_out = new socket_t(*inp_context.getContext(), ZMQ_PUB);

            if (parent == "__bind__") {
                inPoint += name + ".sub";
                outPoint += name + ".pub";
                inp_in->bind(inPoint.c_str());
                inp_out->bind(outPoint.c_str());

                // The binder will listen to our output and route it for us
                subscribe(CHANNEL::Out);
            } else {
                // We sub their pub and v/v
                inPoint += parent + ".pub";
                outPoint += parent + ".sub";
                inp_in->connect(inPoint.c_str());
                inp_out->connect(outPoint.c_str());

                // The connector will listen on a named channel for directed messages
                subscribe(chanToStr[CHANNEL::In]  + chanToStr[CHANNEL::DELIM] + name);
                subscribe(chanToStr[CHANNEL::Cmd] + chanToStr[CHANNEL::DELIM] + name);
            }

            // In and Command are global channels, everyone hears these
            subscribe(CHANNEL::In);
            subscribe(CHANNEL::Cmd);

            _logger.log(name, "Sockets Open!", true);
            _connected = true;
        } catch (const zmq::error_t &e) {
            _logger.err(name, e.what());
        }
    }
};

void Socketer::closeSockets() {
    if (isConnected()) {
        inp_in->close();
        inp_out->close();

        delete inp_in;
        delete inp_out;
        _connected = false;
    }
};

bool Socketer::pollAndProcess() {
    return recvMessage<bool>([&](const Message& msg) {
        // No breaks, all return.
        switch (msg.m_chan) {

            case CHANNEL::None:
			case CHANNEL::DELIM:
                return false;

            case CHANNEL::Cmd:
                //TODO: We should check for nice close messages here?
                return emit("process_command", msg);

            case CHANNEL::In:
                return emit("process_input", msg);

            case CHANNEL::Out:
                return emit("process_output", msg);
        }

        return emit("process_message", msg);
    });
};

void Socketer::on(string hookName, function<bool(const Message&)> callback) {
    processCallbacks[hookName] = callback;
};

bool Socketer::emit(string hookName, const Message& msg) {
    if (processCallbacks.find(hookName) != processCallbacks.end())
        return processCallbacks[hookName](msg);

    throw NonExistantHook(hookName);
}

bool Socketer::isConnected() const {
    return _connected;
};

void Socketer::subscribe(CHANNEL chan) {
    try {
        auto channel = chanToStr[chan];
        inp_in->setsockopt(ZMQ_SUBSCRIBE, channel.data(), channel.size());

    } catch (const zmq::error_t &e) {
        _logger.err(name, e.what());
    }
};

void Socketer::subscribe(const string& chan) {
    try {
        inp_in->setsockopt(ZMQ_SUBSCRIBE, chan.data(), chan.size());
    } catch (const zmq::error_t &e) {
        _logger.err(name, e.what());
    }
};

bool Socketer::sendMessage(Message& message) const {
    bool sendOk = false;

    auto message_string = message.serialise();
    _logger.log(name, "Formatted, before sending ---- " + message_string);

    message_t zmqObject(message_string.length());
    memcpy(zmqObject.data(), message_string.data(), message_string.length());

    try {
        sendOk = inp_out->send(zmqObject);
    } catch (const zmq::error_t &e) {
        _logger.err(name, e.what());
    }

    return sendOk;
};

template<typename retType>
retType Socketer::recvMessage(function<retType(const Message&)> callback, long timeout) {
    pollitem_t pollSocketItems[] = {
        { (void*)*inp_in, 0, ZMQ_POLLIN, 0 }
    };

    message_t zMessage;

    try {
        if (zmq::poll(pollSocketItems, 1, timeout) > 0 && inp_in->recv(&zMessage)) {
            const string normMsg = string(static_cast<char*>(zMessage.data()), zMessage.size());

			Message msg;
			msg.deserialise(normMsg);
            // _logger.log(name, "Normalised, before processing --- " + normMsg);
			// _logger.log(name, "Assigned,   after  processing --- " + msg.serialise());

            return callback(msg);
        }

    } catch (const zmq::error_t &e) {
        _logger.err(name, e.what());

    }

    Message msg;
    return callback(msg);
};

}}
