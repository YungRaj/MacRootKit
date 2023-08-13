#include "SEP.hpp"

#include "MacRootKit.hpp"

namespace SEP
{

void initialize()
{
	MacRootKit *rootkit = mac_rootkit_get_rootkit();

	plugin = new Plugin("SEP", 1, 0, NULL, 0, NULL, 0, NULL, 0);

	appleA7IOP = rootkit->getKextByIdentifier("com.apple.driver.AppleA7IOP");
	appleSEPManager = rootkit->getKextByIdentifier("com.apple.driver.AppleSEPManager");
	appleKeyStore = rootkit->getKextByIdentifier("com.apple.driver.AppleSEPKeyStore");
	appleCredentialManager = rootkit->getKextByIdentifier("com.apple.driver.AppleCredentialManager");

	plugin->addTarget(appleA7IOP);
	plugin->addTarget(appleSEPManager);
	plugin->addTarget(appleKeyStore);
	plugin->addTarget(appleCredentialManager);
}

void installAppleA7IOPHooks()
{

}

void installAppleSEPManagerHooks()
{

}

void installAppleKeyStoreHooks()
{

}

void installAppleCredentialManagerHooks()
{

}

};