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

		void insert(string link) {
			links.push_back(link);
		};

		string current() {
			if (length() > 0)
				return links.at(counter);
			else
				throw 50;
		};

		string next() {
			// counter is 0, length is 1, index is therefore is 0
			// so only increase if we are less then the one less then the length
			if (counter < (length()-1))
				counter++;
			else // when we get there, throw!
				throw 50;

			return current();
		};

		unsigned int length() {
			return links.size();
		};

		Chain* copy() {
			auto newchain = new Chain(*this);
			return newchain;
		};


	private:
		unsigned int counter = 0;
		vector<string> links;
};

}
