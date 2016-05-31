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
				void copyMessage(string in) {
					// cout << in << endl;
					/*
					 * m_chan and m_to are split by '-' to make use of ZMQ's subscriptions.
					 */

					auto h_b = tokeniseString(in, ";");

					// Our channel deets
					auto chans  = tokeniseString(h_b.at(0), "-");

					if (chans.size() == 2) {
						m_to = chans.at(1);
						m_chantype = ChannelType::Directed;
					} else {
						m_chantype = ChannelType::Global;
					}

					m_chan = strToChan[chans.at(0)];


					// Chain deets
					auto chain = tokeniseString(h_b.at(1), ",");
					_chainID   = chain.at(0);
					_chainRef  = chain.at(1);

					// Now all our data
					_data      = h_b.at(2);

					if (h_b.size() > 3) {
						h_b.erase(h_b.begin());
						h_b.erase(h_b.begin());
						h_b.erase(h_b.begin());
						for (auto& tk : h_b)
							_data += ";" + tk;
					}
				};

			protected:
				string  _data    = "";
				string _chainID  = "0";
				string _chainRef = "0";

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

				bool payload(string in) {
					_data = in;
					return true;
				};

				/* This defines how our messages look.
				 * ChannelID ; ChainID,ChainRef ; Data
				 *
				 * Messages should always be "repacked" with data, so they keep a ref count.
				 */
				string serialise(bool increase) {
					if (increase)
						_chainRef = to_string(stoul(_chainRef) + 1);
					return serialise();
				};
				string serialise() const {
					string to = ";";
					if (m_to != "")
						to = "-" + m_to + to;

					return chanToStr[m_chan] + to + _chainID + "," + _chainRef + ";" + _data;
				};

				const Message& deserialise(const string& in) {
					copyMessage(in);
					return *this;
				};
		};
	}
}
