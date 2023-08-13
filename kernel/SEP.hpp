#ifndef __SEP_HPP_
#define __SEP_HPP_

#include "Array.hpp"

#include "Plugin.hpp"

namespace mrk
{
	class MacRootKit;
};

extern mrk::MacRootKit* mac_rootkit_get_rootkit();

namespace SEP
{
	static Plugin *plugin;

	static Kext *appleA7IOP;
	static Kext *appleSEPManager;
	static Kext *appleKeyStore;
	static Kext *appleCredentialManager;
	
	void initialize();

	void installAppleA7IOPHooks();
	void installAppleSEPManagerHooks();
	void installAppleKeyStoreHooks();
	void installAppleCredentialManagerHooks();
};

#endif