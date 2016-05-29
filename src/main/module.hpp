#pragma once
// Common
#include <string>

#include <chrono>
#include <functional>
#include "main/imodule.hpp"
#include "main/logger.hpp"
#include "main/messages/socketer.hpp"
#include "main/exceptions/exceptions.hpp"
#include "main/messages/messages.hpp"

using namespace std;

namespace cppm {
	using namespace messages;

	class Module : public interfaces::IModule {
		private:
			chrono::system_clock::time_point timeNow;

		protected:
			Socketer*  _socketer;
			Logger     _logger;
			ModuleInfo __info;

		public:
			Module(string name, string author);
			virtual ~Module();

			virtual void polltick();
			virtual void tick(){};
			virtual void setup()=0;

			string name() const;
			bool connectToParent(string p, const Context& ctx);
	};
}
