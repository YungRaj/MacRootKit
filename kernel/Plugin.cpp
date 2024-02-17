#include "Plugin.hpp"

using namespace mrk;

Plugin::Plugin(IOService *service,
			    char *product, Size version, UInt32 runmode,
				const char **disableArg, Size disableArgNum,
				const char **debugArg, Size debugArgNum,
				const char **betaArg, Size betaArgNum)
	: service(service),
	  product(product),
	  version(version),
	  runmode(runmode),
	  disableArg(disableArg),
	  disableArgNum(disableArgNum),
	  debugArg(debugArg),
	  debugArgNum(debugArgNum),
	  betaArg(betaArg),
	  betaArgNum(betaArgNum)
{

}

Plugin::Plugin(char *product, Size version, UInt32 runmode,
				const char **disableArg, Size disableArgNum,
				const char **debugArg, Size debugArgNum,
				const char **betaArg, Size betaArgNum)
	: service(NULL),
	  product(product),
	  version(version),
	  runmode(runmode),
	  disableArg(disableArg),
	  disableArgNum(disableArgNum),
	  debugArg(debugArg),
	  debugArgNum(debugArgNum),
	  betaArg(betaArg),
	  betaArgNum(betaArgNum)
{

}