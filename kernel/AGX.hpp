#ifndef __AGX_HPP_
#define __AGX_HPP_

#include "Plugin.hpp"

namespace mrk
{
	class MacRootKit;
};

extern mrk::MacRootKit* mac_rootkit_get_rootkit();

namespace AGX
{
	static Plugin *plugin;

	static Kext *agxG13X;
  	static Kext *agxFirmwareKextRTBuddy64;
  	static Kext *agxFirmwareKextG13XRTBuddy;
	
	void initialize();
};

#endif-