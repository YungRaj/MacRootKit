#ifndef __PLUGIN_HPP_
#define __PLUGIN_HPP_

#include <IOKit/IOService.h>
#include <sys/types.h>

#include "API.h"
#include "APIUtil.hpp"

namespace mrk
{
	struct Plugin
	{
		const char *product;

		size_t version;
		
		uint32_t runmode;
		
		const char **disableArg;
		size_t disableArgNum;
		
		const char **debugArg;
		size_t debugArgNum;

		const char **betaArg;
		size_t betaArgNum;
		
		void (*pluginStart)();
		void (*pluginStop)();
	};

	extern Plugin ADDPR(config);

	extern bool ADDPR(startSuccess);
};


#ifdef MRK_PLUGIN

class EXPORT PRODUCT_NAME : public IOService
{
	OSDeclareDefaultStructors(PRODUCT_NAME)

public:
	IOService *probe(IOService *provider, SInt32 *score) override;
	
	bool start(IOService *provider) override;
	void stop(IOService *provider) override;
};

extern PRODUCT_NAME *ADDPR(selfInstance);

#endif

#endif