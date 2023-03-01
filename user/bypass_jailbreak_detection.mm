#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mach/mach.h> 
#include <mach/exc.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <IOKit/IOKitLib.h>

#if TARGET_OS_MAC

#include <AppKit/AppKit.h>

#endif

#include <Foundation/Foundation.h>

bool swizzled_is_JailBroken(id self, SEL selector)
{
	return NO;
}

bool swizzled_deviceType(id self, SEL selector)
{
	return 1;
}

void swizzleImplementations()
{
	Class cls = objc_getClass("UIDevice");

	SEL originalSelector = @selector(isJailBroken:);
	SEL swizzledSelector = @selector(swizzled_is_JailBroken:);

	BOOL didAddMethod = class_addMethod(cls,
										swizzledSelector,
										(IMP) swizzled_is_JailBroken,
										"@:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	originalSelector = @selector(deviceType:);
	swizzledSelector = @selector(swizzled_deviceType:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP) swizzled_deviceType,
									"@:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}
}

extern "C"
{
	__attribute__((constructor))
	static void initializer()
	{
		printf("[%s] initializer()\n", __FILE__);

		swizzleImplementations();
	}

	__attribute__ ((destructor))
	static void finalizer()
	{
		printf("[%s] finalizer()\n", __FILE__);
	}
}