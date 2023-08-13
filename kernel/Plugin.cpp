#include "Plugin.hpp"

using namespace mrk;

Plugin::Plugin(IOService *service,
			    char *product, size_t version, uint32_t runmode,
				const char **disableArg, size_t disableArgNum,
				const char **debugArg, size_t debugArgNum,
				const char **betaArg, size_t betaArgNum) :
				service(service),
				product(product),
				version(version),
				runmode(runmode),
				disableArg(disableArg) :
				disableArgNum(disableArgNum),
				debugArg(debugArg),
				debugArgNum(debugArgNum),
				betaArg(betaArg),
				betaArgNum(betaArgNum)
{

}

Plugin::Plugin(char *product, size_t version, uint32_t runmode,
				const char **disableArg, size_t disableArgNum,
				const char **debugArg, size_t debugArgNum,
				const char **betaArg, size_t betaArgNum)
				service(NULL),
				product(product),
				version(version),
				runmode(runmode),
				disableArg(disableArg) :
				disableArgNum(disableArgNum),
				debugArg(debugArg),
				debugArgNum(debugArgNum),
				betaArg(betaArg),
				betaArgNum(betaArgNum)
{

}