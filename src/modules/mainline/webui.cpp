#include "modules/mainline/WebUI.hpp"

namespace webui {

WebUIModule::WebUIModule() : Module("WebUI", "Joe Eaves") {
	mg_mgr_init(&manager, this);
	connection = mg_bind(&manager, ":8066", WebUIModule::MongooseCallback);
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
	moduleRunning.payload("module-loaded");
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

void WebUIModule::serveGET(struct mg_connection *nc, int ev, void *ev_data) {

	static const struct mg_str s_get_method = MG_MK_STR("GET");
	struct http_message *hm = (struct http_message *) ev_data;

	// _logger.log(name(), hm->uri.p, true);

	if (is_equal(&hm->method, &s_get_method)) {
		string query(hm->query_string.p, hm->query_string.len);
		auto params = tokeniseString(query, "?");

		if (params.size() > 0) {
			_logger.log(name(), "GET HERE", true);
			mg_printf(nc, "%s{\"message\":\"success!\"}", HTTP_API_SUCCESS);
		} else {
			mg_printf(nc, "%s{}", HTTP_API_FAILURE);
		}
	} else {
		mg_printf(nc, "%s", HTTP_NOT_IMPL);
	}

	nc->flags |= MG_F_SEND_AND_CLOSE;
	// 	//mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */

}

void WebUIModule::MongooseCallback(struct mg_connection *nc, int ev, void *ev_data) {
	WebUIModule* handler = WebUIModule::GetSingleton(nc);
	switch (ev) {
		case MG_EV_HTTP_REQUEST:
			handler->serveGET(nc, ev, ev_data);
			break;
		default:
			break;
	}
}

WebUIModule* WebUIModule::GetSingleton(struct mg_connection *nc) {
	/** This handler will get our instance via mg_connection::mgr::user_data. As the docs say:
		https://docs.cesanta.com/mongoose/dev/#/c-api/net.h/mg_mgr_init/ For C++ example
	**/
	return static_cast<WebUIModule*>(nc->mgr->user_data);
}

// bool WebUIModule::Listen()
// {
// 	mg_register_http_endpoint(connection, "/api/game-data", HandleEndpoint_API_GameData);
// 	mg_register_http_endpoint(connection, "/api/fail-data", HandleEndpoint_API_FailData);
//
// 	mg_set_protocol_http_websocket(connection);
// 	return connection->err == 0;
// }

// void WebUIModule::Send(struct mg_connection *nc, const char* response)
// {
// 	mg_printf(nc, response);
// 	nc->flags |= MG_F_SEND_AND_CLOSE;
// }

}

webui::WebUIModule* createModule() {return new webui::WebUIModule;}
void destroyModule(webui::WebUIModule* module) {delete module;}
