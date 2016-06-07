#include "modules/mainline/WebUI.hpp"

namespace webui {

WebUIModule::WebUIModule() : Module("WebUI", "Joe Eaves") {
	mg_mgr_init(&manager, this);
	connection = mg_bind(&manager, ":8066", WebUIModule::ServeCallback);
	mg_register_http_endpoint(connection, "/api", WebUIModule::APICallback);
	mg_set_protocol_http_websocket(connection);
}

WebUIModule::~WebUIModule() {
	mg_mgr_free(&manager);
}

void WebUIModule::setup() {

	_socketer->on("process_command", [&](const Message& message) {
		_logger.log(name(), message.serialise(), true);
		return true;
	});

	_eventer.on("poll-manager", [&] {
		mg_mgr_poll(&manager, 0);
	}, EventPriority::HIGH);

	Command moduleRunning(name());
	moduleRunning.payload("spine://module/loaded?name=" + name());
	_socketer->sendMessage(moduleRunning);
}

void WebUIModule::tick() {
	_eventer.emit("poll-manager");
	_eventer.emitTimedEvents();
}

int WebUIModule::has_prefix(const struct mg_str *uri, const struct mg_str *prefix) {
  return (uri->len > prefix->len) && (memcmp(uri->p, prefix->p, prefix->len) == 0);
}

int WebUIModule::is_equal(const struct mg_str *s1, const struct mg_str *s2) {
  return (s1->len == s2->len) && (memcmp(s1->p, s2->p, s2->len) == 0);
}

void WebUIModule::APICallback(struct mg_connection *nc, int ev, void *ev_data) {
	WebUIModule* handler = WebUIModule::GetSingleton(nc);
	static const struct mg_str s_get_method = MG_MK_STR("GET");
	struct http_message *hm = (struct http_message *) ev_data;

	// _logger.log(name(), hm->uri.p, true);

	if (handler->is_equal(&hm->method, &s_get_method)) {
		string query;
		char decoded[hm->query_string.len+1];
		auto d_count = mg_url_decode(hm->query_string.p, hm->query_string.len, &decoded[0], sizeof(decoded), 0);

		if (d_count >= 0) {
			if (d_count == 0)
				query.assign(hm->query_string.p, hm->query_string.len);
			else
				query.assign(&decoded[0], d_count);

			auto params = tokeniseString(query, "&");

			if (params.size() > 0) {
				auto handleError = handler->handleQuery(params);

				if (handleError == -1) {

					mg_send_response_line(nc, 500, HTTP_API);
					mg_printf(nc, "{\"message\":\"No commands found!\", \"query_echo\":\"%s\"}", query.c_str());

				} else if (handleError == 0) {

					mg_send_response_line(nc, 200, HTTP_API);
					mg_printf(nc, "{\"message\":\"success!\"}");

				} else {

					mg_send_response_line(nc, 500, HTTP_API);
					mg_printf(nc, "{\"message\":\"Errors found for commands\", \"errors\": %d, \"query_echo\":\"%s\"}", handleError, query.c_str());

				}
			} else {
				mg_send_response_line(nc, 500, HTTP_NOT_IMPL);
			}
		} else {
			mg_send_response_line(nc, 500, HTTP_NOT_IMPL);
		}
	} else {
		mg_send_response_line(nc, 500, HTTP_NOT_IMPL);
	}

	nc->flags |= MG_F_SEND_AND_CLOSE;
	// 	//mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */

}

int WebUIModule::handleQuery(vector<string>& params) {
	int foundCommands = 0;
	int failures = 0;

	for (auto& param : params) {

		auto split = tokeniseString(param, "=");

		if (split.at(0) == "commands[]" && split.size() == 2) {
			_logger.log(name(), "[API] commands = " + split.at(1), true);

			Command com(name());
			com.payload(split.at(1));
			if (!_socketer->sendMessage(com))
				failures++;

			foundCommands++;
		}
	}

	return (foundCommands == 0) ? -1 : failures;
}

void WebUIModule::ServeCallback(struct mg_connection *nc, int ev, void *ev_data) {
	WebUIModule* handler = WebUIModule::GetSingleton(nc);
	switch (ev) {
		case MG_EV_HTTP_REQUEST:
			handler->servePage(nc, &pages::INDEX_HTML[0], &pages::INDEX_HTML_SIZE);
			break;
		default:
			break;
	}
}

void WebUIModule::servePage(struct mg_connection* nc, unsigned char* page64, int* size) {
	char html[*size];
	mg_base64_decode(page64, *size, html);
	mg_send_response_line(nc, 200, "Content-Type: text/html\r\n");
	mg_printf(nc, "%s", html)	;
	nc->flags |= MG_F_SEND_AND_CLOSE;
}

WebUIModule* WebUIModule::GetSingleton(struct mg_connection *nc) {
	/** This handler will get our instance via mg_connection::mgr::user_data. As the docs say:
		https://docs.cesanta.com/mongoose/dev/#/c-api/net.h/mg_mgr_init/ For C++ example
	**/
	return static_cast<WebUIModule*>(nc->mgr->user_data);
}

}

webui::WebUIModule* createModule() {return new webui::WebUIModule;}
void destroyModule(webui::WebUIModule* module) {delete module;}
