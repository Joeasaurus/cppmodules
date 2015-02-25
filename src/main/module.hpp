#include <string>
#include <iostream>

#ifndef H_MODULE
#define H_MODULE

typedef struct ModuleInfo {
	std::string name;
	std::string author;
} ModuleInfo;

class Module {
	protected:
		ModuleInfo __info;
	public:
		virtual ~Module(){};
		virtual std::string name()=0;
};

typedef Module* Module_loader(void);
typedef void Module_unloader(Module*);

#endif // H_MODULE