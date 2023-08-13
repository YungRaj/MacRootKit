#ifndef __BCMWLAN_HPP_
#define __BCMWLAN_HPP_

#include "Plugin.hpp"

namespace mrk
{
	class MacRootKit;
};

namespace BCMWLAN
{
	static Plugin *plugin;

	static Kext *appleBCMWLANCore;
	static Kext *appleBCMWLANBusInterfacePCIe;
	static Kext *appleBCMWLANFirmware4387;
	
	void initialize();
};


#endif