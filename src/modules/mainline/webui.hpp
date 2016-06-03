#pragma once
#include "main/module.hpp"
#include "main/messages/messages.hpp"
#include "Eventer.hpp"

using namespace cppm;
using namespace cppm::messages;
using namespace cppevent;

#include "mongoose.h"

namespace webui {
static const char* HTTP_NOT_IMPL = "HTTP/1.0 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
static const char* HTTP_API_SUCCESS = "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\n\r\n";
static const char* HTTP_API_FAILURE = "HTTP/1.0 400 Bad Request\r\nContent-Type: application/json\r\n\r\n";

class WebUIModule : public Module {
	public:
		WebUIModule();
		~WebUIModule();
		void setup();
		void tick();

		static void MongooseCallback(struct mg_connection *nc, int ev, void *p);
		static WebUIModule* GetSingleton(struct mg_connection *nc);
	private:
		Eventer _eventer;
		Command message{"WebUI"};

		struct mg_connection *connection;
		struct mg_mgr manager;

		int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
		int is_equal(const struct mg_str *s1, const struct mg_str *s2);
		void serveGET(struct mg_connection *nc, int ev, void *ev_data);

};

}

// Init/Del functions.
extern "C" CPPMAPI webui::WebUIModule* createModule();
extern "C" CPPMAPI void destroyModule(webui::WebUIModule* module);
