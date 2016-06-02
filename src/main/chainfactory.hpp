#pragma once

#include <climits>
#include <iostream>
#include <string>
#include <map>

using namespace std;
namespace cppm {

class ChainFactory {
	private:
		const unsigned long max_index = ULONG_MAX - 10;
		map<unsigned long, map<unsigned long, Chain*>> moduleChains;

		void insert(unsigned long chainID, unsigned long chainRef, string moduleName) {
			if (has(chainID, chainRef))
				moduleChains[chainID][chainRef]->insert(moduleName);
			else
				throw 50; // invalid copy source
		};

		unsigned long getNextIndex(unsigned long lastIndex) {
			return (lastIndex >= max_index) ? 1 : lastIndex + 1;
		}

	public:
		ChainFactory() {
			Chain* chain1 = new Chain();
			map<unsigned long, Chain*> refChain {
				make_pair(0, chain1)
			};
			moduleChains[0] = refChain; // 0 is the empty copy
		};

		unsigned long create() {
			return create(0, true);
		};

		unsigned long create(unsigned long indexToCopy, bool toplevel = false) {
			unsigned long index;

			// THIS IS WHERE THE CHANGES GO R4wizard

			if (toplevel) {
				map<unsigned long, Chain*> refChain {
					make_pair(0, moduleChains[indexToCopy][0]->copy())
				};

				index = getNextIndex(moduleChains.rbegin()->first);
				moduleChains[index] = refChain;
			} else {
				index = getNextIndex(moduleChains[indexToCopy].rbegin()->first);
				moduleChains[indexToCopy][index] = moduleChains[indexToCopy][0]->copy();
			}

			return index;
		};


		bool has(unsigned long chainID) {
			return has(chainID, 0);
		};

		bool has(unsigned long chainID, unsigned long chainRef) {
			return (moduleChains.count(chainID) && moduleChains[chainID].count(chainRef));
		};

		bool hasEnded(unsigned long chainID, unsigned long chainRef, bool removeDeadChain = false) {
			if (has(chainID, chainRef)) {
				auto result = moduleChains[chainID][chainRef]->ended();
				if (removeDeadChain && result)
					delete moduleChains[chainID][chainRef];
					return moduleChains[chainID].erase(chainRef) > 0;

				return result;
			}

			throw 50; // invalid chainid
		};

		void insert(unsigned long chainID, string moduleName) {
			insert(chainID, 0, moduleName);
		};

		string current(unsigned long chainID, unsigned long chainRef) {
			if (has(chainID, chainRef))
			 	return moduleChains[chainID][chainRef]->current();

			throw 50;
		}

		string next(unsigned long chainID, unsigned long chainRef) {
			if (has(chainID, chainRef))
				return moduleChains[chainID][chainRef]->next();

			throw 50;
		};
};

}
