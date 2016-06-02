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
				void copyMessage(const string& in) {
					/*
					 * m_chan and m_to are split by 'DELIM' to make use of ZMQ's subscriptions.
					 */

					auto header = in.substr(0,in.find(" "));

					// Our channel deets
					auto chans  = tokeniseString(header, chanToStr[CHANNEL::DELIM]);

					if (chans.size() == 2) {
						m_to = chans.at(0);
						m_chan = strToChan[chans.at(1)];
						m_chantype = ChannelType::Directed;
					} else {
						m_chan = strToChan[chans.at(0)];
						m_chantype = ChannelType::Global;
					}

					auto body = tokeniseString(in.substr(in.find(" ") + 1), ";");
					m_from = body.at(0);

					// Chain deets
					auto chain = tokeniseString(body.at(1), ",");
					_chainID   = stoul(chain.at(0));
					_chainRef  = stoul(chain.at(1));

					// Now all our data
					_data      = body.at(2);

					if (body.size() > 3) {
						body.erase(body.begin());
						body.erase(body.begin());
						body.erase(body.begin());
						for (auto& tk : body)
							_data += ";" + tk;
					}
				};

			protected:
				string  _data    = "";
				unsigned long _chainID  = 0;
				unsigned long _chainRef = 0;

			public:
				CHANNEL m_chan         = CHANNEL::None;
				ChannelType m_chantype = ChannelType::Global;
				string  m_to           = "";
				string  m_from         = "";

				Message(){};
				Message(const string& messageLine) {
					deserialise(messageLine);
				};

				Message(const string& from, CHANNEL chan = CHANNEL::In) {
					m_from = from;
					m_chan = chan;
				};

				Message(const string& from, const string& to, CHANNEL chan = CHANNEL::In) : Message(from, chan) {
					m_to = to;
				};

				void sendFrom(const string& from) {
					m_from = from;
				};

				void sendTo(const string& to) {
					m_to = to;
					m_chantype = ChannelType::Directed;
				};

				void setChannel(CHANNEL chan) {
					m_chan = chan;
					if (m_to == "")
						m_chantype = ChannelType::Global;
					else
						m_chantype = ChannelType::Directed;
				};

				void setChain(unsigned long& chainID, unsigned long& chainRef) {
					_chainID  = chainID;
					_chainRef = chainRef;
				};
				void setChain(pair<unsigned int, unsigned int> chains) {
					_chainID  = chains.first;
					_chainRef = chains.second;
				};

				string getChainString() const {
					return to_string(_chainID) + "," + to_string(_chainRef);
				};
				pair<unsigned long, unsigned long> getChain() const {
					return {_chainID, _chainRef};
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
				 */
				string serialise() const {
					string to = "";
					if (m_to != "")
						to = m_to + chanToStr[CHANNEL::DELIM];

					return to + chanToStr[m_chan] + " " + m_from + ";" + getChainString() + ";" + _data;
				};

				const Message& deserialise(const string& in) {
					copyMessage(in);
					return *this;
				};
		};
	}
}
