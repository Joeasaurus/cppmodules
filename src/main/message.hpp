#pragma once

#include <string>
#include <fstream>
#include "json/json.h"

using namespace std;

namespace cppm {
	// Static functions so we don't need an instance of Message
	static string asString(const Json::Value& printable) {
		Json::StreamWriterBuilder wbuilder;
		wbuilder["indentation"] = "";
		return Json::writeString(wbuilder, printable);
	};
	static Json::Value& fromString(Json::Value& value, const string& parsable) {
		Json::Reader reader;
		reader.parse(parsable, value, false);
		return value;
	};

	// Our classes
	class Message {
		protected:
			Json::Value message;
			Json::Reader reader;
			Json::StreamWriterBuilder wbuilder;
		public:
			string CHANNEL = "NONE";
			Message() {
				wbuilder["indentation"] = "";
				message["data"] = Json::objectValue;
				message["source"] = "";
				message["destination"] = "";
			};
			Message(string name, string destination) : Message() {
				message["source"] = name;
				message["destination"] = destination;
			};
			Message(const Message& rhs) : message(rhs.message) {
				wbuilder["indentation"] = "";
			};
			Json::Value& operator[](const string arrayKey) {
				return message[arrayKey];
			};
			const Json::Value& operator[](const string arrayKey) const {
				return message[arrayKey];
			};
		// These are our 'interface'
			virtual Json::Value asJson() const {
				return message;
			}
			virtual string asString() const {
				return Json::writeString(wbuilder, message);
			};
			virtual void fromJson(const Json::Value& inMessage) {
				// I think the const_cast is needed here, but I'm unsure.
				// We'll roll without it until a const message trips us up!
				message = inMessage; //const_cast<Json::Value&>(inMessage);
			};
			virtual void fromString(const string& parsable) {
				reader.parse(parsable, message, false);
			};
			virtual void fromFile(const string& filename) {
				ifstream fileStream(filename);
				if (fileStream) {
					stringstream buffer;
					buffer << fileStream.rdbuf();
					reader.parse(buffer.str(), message, false);
				} else {
					//TODO: Improve this!
					throw 50;
				}
			};
	};
}