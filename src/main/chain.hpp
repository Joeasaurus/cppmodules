#pragma once

#include <string>
#include <vector>

using namespace std;
namespace cppm {

class Chain {
	public:
		Chain() {};
		Chain(Chain& rhs) : links(rhs.links) {};
		Chain(const Chain& rhs) : links(rhs.links) {};

		const string nullchar = "$$_CHAIN__NULL_$$";

		void insert(string link) {
			links.push_back(link);
		};

		string current() {
			if (length() > 0)
				return links.at(counter);
			else
				return nullchar;
		};

		string next() {
			// counter is 0, length is 1, index is therefore is 0
			// so only increase if we are less then the one less then the length
			if (ended()) return nullchar;

			counter++;
			return current();
		};

		unsigned long length() {
			return links.size();
		};

		Chain* copy() {
			auto newchain = new Chain(*this);
			return newchain;
		};

		bool ended() {
			return counter == length() - 1;
		};

	private:
		unsigned long counter = 0;
		vector<string> links;
};

}
