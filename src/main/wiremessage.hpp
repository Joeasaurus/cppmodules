#pragma once

#include <string>
#include <fstream>
#include "json/json.h"

using namespace std;

class WireMessage {
	private:
		Json::Value message;
		Json::Reader reader;
		Json::StreamWriterBuilder wbuilder;
	public:
		WireMessage() {
			message["data"] = Json::objectValue;
			message["source"] = "";
			message["destination"] = "";
			wbuilder["indentation"] = "";
		};
		WireMessage(string name, string destination) {
			message["data"] = Json::objectValue;
			message["source"] = name;
			message["destination"] = destination;
			wbuilder["indentation"] = "";
		};
		WireMessage(const WireMessage& rhs) : message(rhs.message){};
		Json::Value getMessage() const {
			return message;
		}
		void setMessage(const Json::Value& inMessage) {
			// I think the const_cast is needed here, but I'm unsure.
			// We'll roll without it until a const message trips us up!
			message = inMessage; //const_cast<Json::Value&>(inMessage);
		};
		Json::Value& operator[](const string arrayKey) {
	        return message[arrayKey];
	    };
		const Json::Value& operator[](const string arrayKey) const {
	        return message[arrayKey];
	    };
	    string asString(const Json::Value& printable) {
			return Json::writeString(wbuilder, printable);
	    };
	    string asString() const {
	    	return Json::writeString(wbuilder, message);
	    };
	    void parseIn(const string& parsable) {
	    	reader.parse(parsable, message, false);
	    };
	    void parseIn(Json::Value& value, const string& parsable) {
	    	reader.parse(parsable, value, false);
	    };
	    void parseInFile(const string& parsable) {
	    	parseInFile(message, parsable);
	    };
	    void parseInFile(Json::Value& value, const string& parsable) {
	    	ifstream fileStream(parsable);
	    	if (fileStream) {
				stringstream buffer;
				buffer << fileStream.rdbuf();
		    	reader.parse(buffer.str(), value, false);
	    	} else {
	    		//TODO: Improve this!
	    		throw 50;
	    	}
	    };
};