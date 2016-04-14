#pragma once

#include <string>
#include "main/messages/channels.hpp"

using namespace std;

namespace cppm {
	namespace messages {

		static vector<string> tokeniseString(const string& message, const string& spchar) {
			vector<string> messageTokens;
			if (!message.empty()) {
				boost::split(messageTokens, message, boost::is_any_of(spchar));
			}

			return messageTokens;
		};

		class Message {
			private:
				string splitHeadAndData(string in) {
					auto h_b = tokeniseString(in, ";");

					auto header = h_b.at(0);

					if (h_b.size() > 2) {
						h_b.erase(h_b.begin());

						for (auto& tk : h_b) 
							data += " " + tk;
					}

					return header;
				};

			protected:
				string  data  = "";

			public:
				CHANNEL _chan = CHANNEL::None;
				string  _to   = "";
				string  _from = "";

				Message(){};

				Message(const string& from, CHANNEL chan = CHANNEL::In) {
					_from = from;
					_chan = chan;
				};

				Message(const string& from, const string& to, CHANNEL chan = CHANNEL::In) : Message(from, chan) {
					_to = to;
				};

				void sendTo(const string& to) {
					_to = to;
				};

				string payload() const {
					return data;
				};

				bool payload(string in, bool data_only = true) {
					if (data_only) {
						data = in;
						return true;
					}

					/* We know that format sticks a ; between the header and body.
					 * This should be the first colon, so channel, to and from cannot contain one.
					 * We split the string on ; and bung everything right of it into 'data'
					 * The header is then checked for the channel, from and any specific routing (to).
					 */

					try {
						auto fields = tokeniseString(splitHeadAndData(in), " ");
						_chan       = strToChan[fields.at(0)];
						_from       = fields.at(1);
						
						if (fields.size() > 2)
							_to = fields.at(2);

					} catch (exception& e) {
						cout << e.what() << endl;
						return false;
					}

					return true;
				};

				string format() const {
					return chanToStr[_chan] + " " + _to + " " + _from + "; " + data;
				};
		};
	}
}