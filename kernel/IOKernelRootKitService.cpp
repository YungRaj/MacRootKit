/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "IOKernelRootKitService.hpp"
#include "IOKernelRootKitUserClient.hpp"

#include "MacRootKit.hpp"

#include "Kernel.hpp"

#include "Kext.hpp"
#include "KextMachO.hpp"

#include "Log.hpp"

static bool loaded = false;

OSDefineMetaClassAndStructors(IOKernelRootKitService, IOService)

    bool IOKernelRootKitService::init(OSDictionary* properties) {
    this->userClients = OSSet::withCapacity(1);

    if (!this->userClients)
        return false;

    MAC_RK_LOG("MacRK::IOKernelRootKitService::init()!\n");

    return IOService::init(properties);
}

void IOKernelRootKitService::free() {
    IOService::free();
}

bool IOKernelRootKitService::start(IOService* provider) {
    bool keep_syms = false;

    kern_return_t ret = kIOReturnSuccess;

    if (loaded) {
        IOService::stop(provider);

        return !loaded;
    }

    PE_parse_boot_argn("keepsyms", &keep_syms, sizeof(keep_syms));

    loaded = true;

    if (keep_syms) {
        xnu::Mach::VmAddress kernel_base = xnu::Kernel::findKernelBase();

        UInt64 kernel_slide = xnu::Kernel::findKernelSlide();

        char buffer[128];

        MAC_RK_LOG("MacRK::IOKernelRootKitService::start()!\n");

        snprintf(buffer, 128, "0x%llx", kernel_base);

        MAC_RK_LOG("MacRK::IOKernelRootKitService::kernel_base = %s\n", buffer);

        snprintf(buffer, 128, "0x%llx", kernel_slide);

        MAC_RK_LOG("MacRK::IOKernelRootKitService::kernel_slide = %s\n", buffer);

        snprintf(buffer, 128, "0x%x", *(UInt32*)kernel_base);

        MAC_RK_LOG("MacRK::@ kernel base = %s\n", buffer);

        if (kernel_base && kernel_slide) {
            this->kernel = xnu::Kernel::create(kernel_base, kernel_slide);

            this->kernel->setRootKitService(this);

            this->tfp0 = this->kernel->getKernelTaskPort();

            ret = mac_rootkit_start(this, this->kernel, &this->rootkitKext);

            if (ret == kIOReturnSuccess) {
                this->rootkit = mac_rootkit_get_rootkit();
            }

            registerService();
        }
    } else {
        MAC_RK_LOG("MacRK::failed to load! Please enable keepsyms=1 as a boot-arg in NVRAM!\n");

        return kIOReturnUnsupported;
    }

    return ret == kIOReturnSuccess && IOService::start(provider);
}

void IOKernelRootKitService::stop(IOService* provider) {
    kern_return_t ret;

    ret = mac_rootkit_stop(this, this->kernel, &this->rootkitKext);

    if (ret != KERN_SUCCESS) {
        return;
    }

    if (userClients) {
        this->detachUserClients();
    }

    IOService::stop(provider);
}

IOService* IOKernelRootKitService::probe(IOService* provider, SInt32* score) {
    return IOService::probe(provider, score);
}

void IOKernelRootKitService::clientClosed(IOUserClient* client) {
    if (client) {
        this->removeUserClient(reinterpret_cast<IOKernelRootKitUserClient*>(client));
    }
}

IOReturn IOKernelRootKitService::createUserClient(task_t task, void* securityID, UInt32 type,
                                                  IOKernelRootKitUserClient** client) {
    IOReturn result = kIOReturnSuccess;

    IOKernelRootKitUserClient* userClient;

    userClient = IOKernelRootKitUserClient::rootKitUserClientWithKernel(this->kernel, task,
                                                                        securityID, type);

    if (userClient)
        *client = userClient;
    else
        result = kIOReturnNoMemory;

    return result;
}

IOReturn IOKernelRootKitService::createUserClient(task_t task, void* securityID, UInt32 type,
                                                  OSDictionary* properties,
                                                  IOKernelRootKitUserClient** client) {
    IOReturn result = kIOReturnSuccess;

    IOKernelRootKitUserClient* userClient;

    userClient = IOKernelRootKitUserClient::rootKitUserClientWithKernel(
        this->kernel, task, securityID, type, properties);

    if (userClient)
        *client = userClient;
    else
        result = kIOReturnNoMemory;

    return result;
}

IOReturn IOKernelRootKitService::newUserClient(task_t task, void* securityID, UInt32 type,
                                               OSDictionary* properties, IOUserClient** client) {
    IOReturn result;

    IOKernelRootKitUserClient* userClient;

    if (!isInactive()) {
        result = this->createUserClient(task, securityID, type, properties, &userClient);

        if ((result == kIOReturnSuccess) && (userClient != NULL)) {
            if (!reinterpret_cast<IOService*>(userClient)->attach(this)) {
                result = kIOReturnError;
            } else if (!userClient->start(this)) {
                reinterpret_cast<IOService*>(userClient)->detach(this);

                result = kIOReturnError;
            } else {
                userClients->setObject((OSObject*)userClient);
            }

            *client = reinterpret_cast<IOUserClient*>(userClient);
        }
    } else {
        result = kIOReturnNoDevice;
    }

    return result;
}

IOReturn IOKernelRootKitService::newUserClient(task_t task, void* securityID, UInt32 type,
                                               IOUserClient** client) {
    IOReturn result;

    IOKernelRootKitUserClient* userClient;

    if (!isInactive()) {
        result = this->createUserClient(task, securityID, type, &userClient);

        if ((result == kIOReturnSuccess) && (userClient != NULL)) {
            if (!reinterpret_cast<IOService*>(userClient)->attach(this)) {
                result = kIOReturnError;
            } else if (!userClient->start(this)) {
                reinterpret_cast<IOService*>(userClient)->detach(this);

                result = kIOReturnError;
            } else {
                userClients->setObject((OSObject*)userClient);
            }

            *client = reinterpret_cast<IOUserClient*>(userClient);
        }
    } else {
        result = kIOReturnNoDevice;
    }

    return result;
}

IOReturn IOKernelRootKitService::addUserClient(IOKernelRootKitUserClient* client) {
    IOReturn result = kIOReturnSuccess;

    if (!isInactive()) {
        if (!reinterpret_cast<IOService*>(client)->attach(this)) {
            result = kIOReturnError;
        } else if (!client->start(this)) {
            reinterpret_cast<IOService*>(client)->detach(this);

            result = kIOReturnError;
        } else {
            userClients->setObject((OSObject*)client);
        }
    } else {
        result = kIOReturnNoDevice;
    }

    return result;
}

IOReturn IOKernelRootKitService::removeUserClient(IOKernelRootKitUserClient* client) {
    IOService* userClient = dynamic_cast<IOService*>(client);

    userClient->retain();

    userClients->removeObject((OSObject*)userClient);

    if (!isInactive()) {
        userClient->terminate();
    }

    userClient->release();

    return kIOReturnSuccess;
}

IOReturn IOKernelRootKitService::detachUserClients() {
    IOReturn result = kIOReturnSuccess;

    if (!isInactive()) {
        OSIterator* iterator;

        iterator = OSCollectionIterator::withCollection(userClients);

        if (iterator) {
            IOKernelRootKitUserClient* client;

            while ((client = (IOKernelRootKitUserClient*)iterator->getNextObject())) {
                reinterpret_cast<IOService*>(client)->terminate();
            }

            iterator->release();
        }
    }

    userClients->flushCollection();

    return result;
}
