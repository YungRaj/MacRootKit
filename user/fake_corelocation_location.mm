#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <sys/un.h>

#include <mach/mach.h> 
#include <mach/exc.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <IOKit/IOKitLib.h>

#ifdef TARGET_OS_IPHONE

#include <UIKit/UIKit.h>

#elif TARGET_OS_MAC

#include <AppKit/AppKit.h>

#endif

#include <Foundation/Foundation.h>
#include <CoreLocation/CoreLocation.h>

id swizzled_initWithLatitudeLongitude(id self_, CLLocationDegrees latitude, CLLocationDegrees longitude)
{
	id obj = objc_msgSend(self_, @selector(swizzled_initWithLatitude:longitude:), 35.652832, 139.839478);

	return obj;
}

id swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyTimestamp(id self_, CLLocationCoordinate2D coordinate, CLLocationDistance altitude, CLLocationAccuracy horizontalAccuracy, CLLocationAccuracy verticalAccuracy, NSDate *timestamp)
{
	CLLocationCoordinate2D adjustedCoordinate = { 35.652832, 139.839478 };

	id obj = objc_msgSend(self_, @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:timestamp:),
						  adjustedCoordinate, altitude, horizontalAccuracy, verticalAccuracy, timestamp);

	return obj;
}

id swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseSpeedTimestamp(id self_, CLLocationCoordinate2D coordinate, CLLocationDistance altitude, CLLocationAccuracy horizontalAccuracy, CLLocationAccuracy verticalAccuracy, CLLocationDirection course, CLLocationSpeed speed, NSDate *timestamp)
{
	CLLocationCoordinate2D adjustedCoordinate = { 35.652832, 139.839478 };

	id obj = objc_msgSend(self_, @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:speed:timestamp:),
						  adjustedCoordinate, altitude, horizontalAccuracy, verticalAccuracy, course, speed, timestamp);

	return obj;
}

id swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseCourseAccuracySpeedSpeedAccuracyTimestamp(id self_, CLLocationCoordinate2D coordinate, CLLocationDistance altitude, CLLocationAccuracy horizontalAccuracy, CLLocationAccuracy verticalAccuracy, CLLocationDirection course, CLLocationDirectionAccuracy courseAccuracy, CLLocationSpeed speed, CLLocationSpeedAccuracy speedAccuracy, NSDate *timestamp)
{
	CLLocationCoordinate2D adjustedCoordinate = { 35.652832, 139.839478 };

	id obj = objc_msgSend(self_, @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:),
						  adjustedCoordinate, altitude, horizontalAccuracy, verticalAccuracy, course, courseAccuracy, speed, speedAccuracy, timestamp);

	return obj;
}

id swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseCourseAccuracySpeedSpeedAccuracyTimestampSourceInfo(id self_, CLLocationCoordinate2D coordinate, CLLocationDistance altitude, CLLocationAccuracy horizontalAccuracy, CLLocationAccuracy verticalAccuracy, CLLocationDirection course, CLLocationDirectionAccuracy courseAccuracy, CLLocationSpeed speed, CLLocationSpeedAccuracy speedAccuracy, NSDate *timestamp, CLLocationSourceInformation *sourceInfo)
{
	CLLocationCoordinate2D adjustedCoordinate = { 35.652832, 139.839478 };

	id obj = objc_msgSend(self_, @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:sourceInfo:),
						  adjustedCoordinate, altitude, horizontalAccuracy, verticalAccuracy, course, courseAccuracy, speed, speedAccuracy, timestamp, sourceInfo);

	return obj;
}

void swizzleImplementations()
{
	Class cls = objc_getClass("CLLocation");

	SEL originalSelector = @selector(initWithLatitude:longitude:);
	SEL swizzledSelector = @selector(swizzled_initWithLatitude:longitude:);

	BOOL didAddMethod = class_addMethod(cls,
										swizzledSelector,
										(IMP) swizzled_initWithLatitudeLongitude,
										"d:d");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	originalSelector = @selector(initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:timestamp:);
	swizzledSelector = @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:timestamp:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP)  swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyTimestamp,
									"^v:d:d:d:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	originalSelector = @selector(initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:speed:timestamp:);
	swizzledSelector = @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:speed:timestamp:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP)  swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseSpeedTimestamp,
									"^v:d:d:d:d:d:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	originalSelector = @selector(initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:);
	swizzledSelector = @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP)  swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseCourseAccuracySpeedSpeedAccuracyTimestamp,
									"^v:d:d:d:d:d:d:d:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	originalSelector = @selector(initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:sourceInfo:);
	swizzledSelector = @selector(swizzled_initWithCoordinate:altitude:horizontalAccuracy:verticalAccuracy:course:courseAccuracy:speed:speedAccuracy:timestamp:sourceInfo:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP)  swizzled_initWithCoordinateAltitudeHorizontalAccuracyVerticalAccuracyCourseCourseAccuracySpeedSpeedAccuracyTimestampSourceInfo,
									"^v:d:d:d:d:d:d:d:@:@");

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