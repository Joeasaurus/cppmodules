#pragma once

#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "main/messages/channels.hpp"

using namespace std;

namespace cppm {
	namespace messages {

		enum ChannelType {
			Global,
			Directed
		};

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
					_data       = h_b.at(1);

					if (h_b.size() > 2) {
						h_b.erase(h_b.begin());
						h_b.erase(h_b.begin());
						for (auto& tk : h_b)
							_data += ";" + tk;
					}

					return header;
				};

			protected:
				string  _data  = "";

			public:
				CHANNEL m_chan         = CHANNEL::None;
				ChannelType m_chantype = ChannelType::Global;
				string  m_to           = "";
				string  m_from         = "";

				Message(){};

				Message(const string& from, CHANNEL chan = CHANNEL::In) {
					m_from = from;
					m_chan = chan;
				};

				Message(const string& from, const string& to, CHANNEL chan = CHANNEL::In) : Message(from, chan) {
					m_to = to;
				};

				void sendTo(const string& to) {
					m_to = to;
				};

				string payload() const {
					return _data;
				};

				bool payload(string in, bool data_only = true) {
					if (data_only) {
						_data = in;
						return true;
					}

					/* We know that format sticks a ; between the header and body.
					 * This should be the first colon, so channel, to and from cannot contain one.
					 * We split the string on ; and bung everything right of it into 'data'
					 * The header is then checked for the m_chan, m_to and from.
					 * m_chan and m_to are split by '-' to make use of ZMQ's subscriptions.
					 */

					try {
						auto fields = tokeniseString(splitHeadAndData(in), " ");
						auto chans  = tokeniseString(fields.at(0), "-");

						if (chans.size() == 2) {
							m_to = chans.at(1);
							m_chantype = ChannelType::Directed;
						} else {
							m_chantype = ChannelType::Global;
						}

						m_chan = strToChan[chans.at(0)];
						m_from = fields.at(1);
					} catch (exception& e) {
						cout << e.what() << endl;
						return false;
					}

					return true;
				};

				string format() const {
					string to = " ";
					if (m_to != "")
						to   = "-" + m_to + to;

					return chanToStr[m_chan] + to + m_from + ";" + _data;
				};
		};
	}
}
