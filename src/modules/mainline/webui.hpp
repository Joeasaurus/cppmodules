#pragma once
#include <map>
#include "main/module.hpp"
#include "main/messages/messages.hpp"
#include "Eventer.hpp"

using namespace std;
using namespace cppm;
using namespace cppm::messages;
using namespace cppevent;

#include "mongoose.h"

namespace webui {
static const char* HTTP_NOT_IMPL = "Not Implemented\r\nContent-Length: 0\r\n";
static const char* HTTP_API      = "Content-Type: application/json\r\n";
static const char* HTTP_HTML     = "Content-Type: text/html\r\n";

class WebUIModule : public Module {
	public:
		WebUIModule();
		~WebUIModule();
		void setup();
		void tick();

		static void ServeCallback(struct mg_connection* nc, int ev, void* p);
		static void APICallback(struct mg_connection* nc, int ev, void* p);
		static WebUIModule* GetSingleton(struct mg_connection *nc);
	private:
		Eventer _eventer;
		map<string, function<bool(MUri& command)>> commands;

		struct mg_connection *connection;
		struct mg_mgr manager;

		int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);
		int is_equal(const struct mg_str *s1, const struct mg_str *s2);

		int handleQuery(vector<string>& params);
		void servePage(struct mg_connection* nc, unsigned char* page64, int* size);

};

}

// Init/Del functions.
extern "C" CPPM_WINEXPORT webui::WebUIModule* createModule();
extern "C" CPPM_WINEXPORT void destroyModule(webui::WebUIModule* module);


namespace webui { namespace pages {
	static unsigned char INDEX_HTML[] = "PCFET0NUWVBFIGh0bWw+DQo8aHRtbCBsYW5nPSJlbiI+DQo8aGVhZD4NCgk8bWV0YSBjaGFyc2V0PSJVVEYtOCI+DQoJPHRpdGxlPkFQSTwvdGl0bGU+DQoJPHN0eWxlIHR5cGU9InRleHQvY3NzIj4NCgkJKiB7DQoJCQlib3gtc2l6aW5nOiBib3JkZXItYm94Ow0KCQl9DQoNCgkJYm9keSB7DQoJCQlmb250LWZhbWlseTogc2Fucy1zZXJpZjsNCgkJfQ0KDQoJCWZvcm0gew0KCQkJd2lkdGg6IDYwMHB4Ow0KCQkJbWFyZ2luOiAwIGF1dG87DQoJCX0NCg0KCQlidXR0b24gew0KCQkJcGFkZGluZzogMC41ZW07DQoJCQliYWNrZ3JvdW5kOiAjZGRkOw0KCQkJYm9yZGVyOiAxcHggc29saWQgI2NjYzsNCgkJfQ0KCQlpbnB1dCB7DQoJCQlwYWRkaW5nOiAwLjVlbTsNCgkJCWJhY2tncm91bmQ6ICNkZGQ7DQoJCQlib3JkZXI6IDFweCBzb2xpZCAjY2NjOw0KCQl9DQoNCgkJLmNvbW1hbmQgaW5wdXQgew0KCQkgIHdpZHRoOiBjYWxjKDEwMCUgLSAzZW0pOw0KCQl9DQoNCgkJLmNvbW1hbmQgYnV0dG9uIHsNCgkJICB3aWR0aDogM2VtOw0KCQkgIGJvcmRlci1sZWZ0OiAwOw0KCQl9DQoNCgkJYnV0dG9uW3R5cGU9c3VibWl0XSB7DQoJCQl3aWR0aDogMTAwJTsNCgkJfQ0KCTwvc3R5bGU+DQo8L2hlYWQ+DQo8Ym9keT4NCgk8Zm9ybSBhY3Rpb249Ii9hcGkiIG1ldGhvZD0iZ2V0Ij4NCgkJPGRpdiBjbGFzcz0iY29tbWFuZHMiPg0KCQk8L2Rpdj4NCg0KCQk8ZGl2IGNsYXNzPSJjb21tYW5kIGNvbW1hbmQtbmV3Ij4NCgkJCTxpbnB1dCB0eXBlPSJ0ZXh0IiBuYW1lPSJjb21tYW5kc1tdIj48YnV0dG9uPis8L2J1dHRvbj4NCgkJPC9kaXY+DQoNCgkJPGJyIC8+DQoNCgkJPGJ1dHRvbiBjbGFzcz0ic2hvcnRjdXQiIGRhdGEtY29tbWFuZD0ic3BpbmU6Y2xvc2UtbW9kdWxlcyI+Q2xvc2UgTW9kdWxlczwvYnV0dG9uPg0KCQk8YnV0dG9uIGNsYXNzPSJzaG9ydGN1dCIgZGF0YS1jb21tYW5kPSJzcGludDplY2hvOkhlbGxvLCBXb3JsZCI+RWNobyBUZXN0PC9idXR0b24+DQoNCgkJPGJyIC8+PGJyIC8+DQoNCgkJPGJ1dHRvbiB0eXBlPSJzdWJtaXQiPnNlbmQ8L2J1dHRvbj4NCgk8L2Zvcm0+DQoNCgk8c2NyaXB0IHNyYz0iaHR0cHM6Ly9hamF4Lmdvb2dsZWFwaXMuY29tL2FqYXgvbGlicy9qcXVlcnkvMS4xMi40L2pxdWVyeS5taW4uanMiPjwvc2NyaXB0Pg0KCTxzY3JpcHQgdHlwZT0idGV4dC9qYXZhc2NyaXB0Ij4NCgkJJChmdW5jdGlvbigpIHsNCgkJCSQoJy5jb21tYW5kLW5ldyBidXR0b24nKS5jbGljayhmdW5jdGlvbihlKSB7DQoJCQkJZS5wcmV2ZW50RGVmYXVsdCgpOw0KCQkJCXZhciAkbmV3ID0gJCgnLmNvbW1hbmQtbmV3JykuY2xvbmUoKTsNCgkJCQkkbmV3LnJlbW92ZUNsYXNzKCJjb21tYW5kLW5ldyIpOw0KCQkJCSRuZXcuZmluZCgnYnV0dG9uJykudGV4dCgnLScpLnVuYmluZCgpLmNsaWNrKGZ1bmN0aW9uKGUpIHsNCgkJCQkJJCh0aGlzKS5uZWFyZXN0KCcuY29tbWFuZCcpLnJlbW92ZSgpOw0KCQkJCX0pOw0KDQoJCQkJJCgnLmNvbW1hbmRzJykuYXBwZW5kKCRuZXcpOw0KCQkJCSQoJy5jb21tYW5kLW5ldyBpbnB1dCcpLnZhbCgiIikuZm9jdXMoKTsNCgkJCX0pOw0KDQoJCQkkKCcuc2hvcnRjdXQnKS5jbGljayhmdW5jdGlvbihlKSB7DQoJCQkJZS5wcmV2ZW50RGVmYXVsdCgpOw0KCQkJCSQoJy5jb21tYW5kLW5ldyBpbnB1dCcpLnZhbCgkKHRoaXMpLmRhdGEoImNvbW1hbmQiKSk7DQoJCQl9KQ0KCQl9KQ0KCTwvc2NyaXB0Pg0KPC9ib2R5Pg0KPC9odG1sPg0K";
	static int INDEX_HTML_SIZE = sizeof(INDEX_HTML);
}}
