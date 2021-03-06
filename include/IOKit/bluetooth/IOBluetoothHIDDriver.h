/*
 * Copyright (c) 2021 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*!
 *   @header IOBluetoothHIDDriver.h
 *   This header contains the definition of the IOBluetoothHIDDriver class, which is the driver for generic Bluetooth HID devices. It is a direct subclass of IOHIDDevice.
 */

#ifndef _IOKIT_IOBLUETOOTHHIDDRIVER_H
#define _IOKIT_IOBLUETOOTHHIDDRIVER_H

#import <IOKit/bluetooth/Bluetooth.h>
#import <IOKit/hid/IOHIDDevice.h>

#import <IOKit/pwr_mgt/RootDomain.h>

#define kBSKernelMonitor2Notification 'bsk2'

class IOTimerEventSource;
class IOWorkLoop;

class IOBluetoothL2CAPChannel;
class IOBluetoothDevice;
class IOWorkQueue;

class IOBluetoothHIDDriver : public IOHIDDevice
{
    OSDeclareDefaultStructors(IOBluetoothHIDDriver)

protected:
    IOWorkLoop *    _workLoop;
    IOCommandGate * _commandGate;

    IOWorkQueue * _desyncWorkQueue;

    IOBluetoothL2CAPChannel * _controlChannel;
    IOBluetoothL2CAPChannel * _interruptChannel;

    IOBluetoothDevice * _device;

    IOMemoryDescriptor * _memDescriptor;
    IOMemoryDescriptor * _getReportDescriptor;
    IONotifier *         _interruptOpenNotification;
    IOTimerEventSource * _timer;
    IONotifier *         _sleepWakeNotifier;

    bool _deviceReady;

    UInt8 _expectedReportID;
    UInt8 _expectedReportType;
    UInt8 _handshake;

    OSDictionary * _deviceProperties;

    UInt16   _vendorIDSource;
    UInt16   _vendorID;
    UInt16   _productID;
    UInt16   _deviceVersion;
    uint32_t _classOfDevice;
    UInt16   _countryCode;

    BluetoothDeviceAddress _deviceAddress;
    char                   _deviceAddressString[20];

    uint32_t _outstandingIO;
    bool     _sendOutstanding;

    // Debug / Behavior Modifiers
    UInt8 _verboseLevel;
    bool  _logPackets;
    bool  _decodePackets;
    bool  _logOutstandingIO;
    bool  _suppressDisconnectNotifications;
    bool  _suppressSetProtocol;
    bool  _driverIsAwake;
    bool  _reservedFlag4;
    UInt8 _reservedByte;

    struct ExpansionData
    {
        OSArray * _sendQueue;

        uint8_t * interruptBuffer;
        uint32_t  interruptBufferUsed;

        uint8_t * controlBuffer;
        uint32_t  controlBufferUsed;

        uint8_t deviceSupportsSuspend;

        uint32_t getReportTimeoutMS;
        uint32_t setReportTimeoutMS;

        uint32_t outstandingMemoryBlockCount;
        bool     waitingForMemoryBlockCount;

        IOPMrootDomain *      fRootDomain;
        IOPMDriverAssertionID fNoDeepSleepAssertionId;

        bool mCommandGateCreated;
        bool mCommandGateAdded;
        bool mControlChannelRetained;
        bool mWorkLoopRetained;

        bool mCloseDownServicesCalled;

        bool mGotNoDeepSleepAssertionID;

        OSString * disconnectionNotificationString;
        OSString * connectionNotificationString;

        IOTimerEventSource * deviceConnectTimer;

        bool mNeedToDropData;

        UInt32 mWakeTime;

        UInt32 mDriverLoadTime;

        IOTimerEventSource * mReadyToSleepTimer;

        bool mHandleStopBeingCalled;

        bool mHandleStartCompleted;

        bool mHIDSuspendSent;

        IOReturn mExitHIDSuspendResult;
    };
    ExpansionData * _expansionData;

public:
    // Standard IOService Methods
    virtual IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;
    virtual bool        init(OSDictionary * properties) APPLE_KEXT_OVERRIDE;
    virtual void        free() APPLE_KEXT_OVERRIDE;
    virtual bool        willTerminate(IOService * provider, IOOptionBits options) APPLE_KEXT_OVERRIDE;

    // Starting & Stopping
    virtual bool handleStart(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void handleStop(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void deviceReady();
    virtual void closeDownServices();

    // Power Management
    virtual void handleSleep();
    virtual void handleWake();
    virtual void handleShutdown(); // Does nothing
    virtual void handleRestart();  // Does nothing

    // HID Properties
    virtual OSString * newTransportString() const APPLE_KEXT_OVERRIDE;
    virtual OSString * newManufacturerString() const APPLE_KEXT_OVERRIDE;
    virtual OSString * newProductString() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newVendorIDSourceNumber() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newVendorIDNumber() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newProductIDNumber() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newVersionNumber() const APPLE_KEXT_OVERRIDE;
    virtual IOReturn   newReportDescriptor(IOMemoryDescriptor ** descriptor) const APPLE_KEXT_OVERRIDE;
    virtual OSString * newSerialNumberString() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newLocationIDNumber() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newCountryCodeNumber() const APPLE_KEXT_OVERRIDE;
    virtual OSNumber * newReportIntervalNumber() const APPLE_KEXT_OVERRIDE;

    // Main UserLand Entry Points
    virtual IOReturn getReport(IOMemoryDescriptor * report, IOHIDReportType reportType, IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setReport(IOMemoryDescriptor * report, IOHIDReportType reportType, IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;

    // General IO
    virtual IOReturn sendData(IOBluetoothL2CAPChannel * theChannel, void * theData, IOByteCount theSize);
    virtual void     processControlData(UInt8 * buffer, UInt16 length);
    virtual void     processInterruptData(UInt8 * buffer, UInt16 length);
    virtual IOReturn waitForData(IOMemoryDescriptor * report, UInt8 btReportType, UInt8 reportID);
    virtual IOReturn waitForHandshake();

    // HID Transaction Methods
    virtual IOReturn hidControl(UInt8 controlOperation);
    virtual int      getProtocol();
    virtual IOReturn setProtocol(UInt8 protocol);
    virtual int      getIdle();
    virtual IOReturn setIdle(UInt8 idleRate);

    // Device Introspection
    virtual bool isKeyboard();
    virtual bool isMouse();

    // Misc
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice) APPLE_KEXT_OVERRIDE;
    virtual IOReturn createCommandGate(IOService * provider);
    virtual IOReturn getDeviceProperties(IOService * provider);
    virtual bool     readDeviceName();

    // Command Gate Actions
    static IOReturn staticCloseDownServicesAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    static IOReturn staticSendToAction(OSObject * owner, void * theChannel, void * theData, void * theSize, void *);
    static IOReturn staticPrepControlChannelAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    static IOReturn staticInterruptChannelOpeningAction(OSObject * owner, void * newService, void * arg2, void * arg3, void * arg4);
    static IOReturn staticWillTerminateAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);

    // Work Loop Methods
    virtual void     closeDownServicesWL();
    virtual IOReturn prepInterruptChannelWL();
    virtual IOReturn getReportWL(IOMemoryDescriptor * report, IOHIDReportType reportType, IOOptionBits options);
    virtual IOReturn setReportWL(IOMemoryDescriptor * report, IOHIDReportType reportType, IOOptionBits options);
    virtual IOReturn processCommandWL(OSString * command, OSNumber * commandParameter);
    virtual IOReturn getDevicePropertiesWL(IOService * provider);
    virtual IOReturn interruptChannelOpeningWL(IOBluetoothL2CAPChannel * theChannel);

    // Timeout Handler
    static void  deviceConnectTimerFired(OSObject * owner, IOTimerEventSource * sender);
    static void  timerFired(OSObject * owner, IOTimerEventSource * sender);
    virtual void handleTimeout();

    // IO Counting
    virtual void incrementOutstandingIO();
    virtual void decrementOutstandingIO();

    // ReadyToSleepTimeout Handler
    static void ReadyToSleepTimerFired(OSObject * owner, IOTimerEventSource * sender);

private:
    // Lazy Interrupt Channel Methods
    static bool     interruptChannelOpeningCallback(void * me, void * ignoreMe, IOService * newService, IONotifier * notifier);
    static IOReturn powerStateHandler(void * target, void * refCon, UInt32 messageType, IOService * service, void * messageArgument, vm_size_t argSize);

public:
    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 0);
    virtual void sendDeviceDisconnectNotifications();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 1);
    virtual IOReturn setPowerStateWL(unsigned long powerStateOrdinal, IOService * whatDevice);

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 2);
    virtual void sendDeviceConnectNotifications();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 3);
    virtual void decrementOutstandingMemoryBlockCount();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 4);
    virtual IOReturn willTerminateWL();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 5);
    virtual void messageClientsWithString(UInt32 type, OSString * message);

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 6);
    virtual void waitForInterruptChannel();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 7);
    virtual void handleStopWL(IOService * provider);

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 8);
    virtual UInt32 GetCurrentTime();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 9);
    virtual void handleReadyToSleepTimerFired();

    OSMetaClassDeclareReservedUsed(IOBluetoothHIDDriver, 10);
    virtual IOReturn HIDCommandSleep(void * event, UInt32 milliseconds, char * calledByFunction, bool panicMachine);

    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 11);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 12);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 13);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 14);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 15);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 16);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 17);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 18);
    OSMetaClassDeclareReservedUnused(IOBluetoothHIDDriver, 19);
};

#endif
