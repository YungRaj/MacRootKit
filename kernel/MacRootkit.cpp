#include "MacRootKit.hpp"

MacRootKit::MacRootKit()
{

}

MacRootKit::~MacRootKit()
{

}

void MacRootKit::registerEntitlementCallback(void *user, entitlement_callback_t callback)
{
	StoredPair<entitlement_callback_t> *pair = new StoredPair<entitlement_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->entitlementCallbacks.add(pair);
}

void MacRootKit::registerBinaryLoadCallback(void *user, binaryload_callback_t callback)
{
	StoredPair<binaryload_callback_t> *pair = new StoredPair<binaryload_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->binaryLoadedCallbacks.add(pair);
}

void MacRootKit::registerKextLoadCallback(void *user, kextload_callback_t callback)
{
	StoredPair<kextload_callback_t> *pair = new StoredPair<kextload_callback_t>;

	pair->first = callback;
	pair->second = user;

	this->entitlementCallbacks.add(pair);
}

void MacRootKit::onEntitlementRequest(task_t task, const char *entitlement, OSObject *original)
{

}

void MacRootKit::onProcLoad(vm_map_t map, const char *path, size_t len)
{

}

void MacRootKit::onKextLoad(Kext *kext, char *kextname)
{
	
}