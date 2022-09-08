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

#include <pthread.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>

#include "Array.hpp"

template<typename classname>
static inline classname getIvar(id self, const char *name)
{
	Ivar ivar(class_getInstanceVariable(object_getClass(self), name));
	
	void *pointer(ivar == NULL ? NULL : reinterpret_cast<char *>(self) + ivar_getOffset(ivar));
	
	return *reinterpret_cast<classname *>(pointer);
}

extern "C"
{
	void init_passbookui_crawl_server(uint16_t port);
	void init_passbookui_crawl_client(int socket);
}

void PKPASVC_viewDidLoad_swizzled(void *self_, SEL cmd_)
{
	UIButton *continuousButton;

	UIView *footerView;

	objc_msgSend((id) self_, @selector(swizzled_viewDidLoad));

	UIView *view = getIvar<UIView*>((id) self_, "_view");

	NSArray *subviews = [[view subviews][0]  subviews];

	for(UIView *subview in subviews)
	{
		if([subview isKindOfClass:objc_getClass("PKPaymentAuthorizationFooterView")])
		{
			footerView = subview;

			break;
		}
	}

	NSLog(@"PassbookUIServiceCrawler::PKPaymentAuthorizationFooterView found!\n");

	if(!footerView)
		return;

	subviews = [footerView subviews];

	for(UIView *subview in subviews)
	{
		if([subview isKindOfClass:objc_getClass("PKContinuousButton")])
		{
			continuousButton = reinterpret_cast<UIButton*>(subview);

			break;
		}
	}

	if(continuousButton)
	{
		NSLog(@"PassbookUIServiceCrawler::PKContinuousButton found! %@\n", continuousButton);

		dispatch_async(dispatch_get_main_queue(), ^
		{
			sleep(4);

			[continuousButton sendActionsForControlEvents:UIControlEventTouchUpInside];
		});
	}
}

void AKMSIVC_viewDidLoad_swizzled(void *self_, SEL cmd_)
{
	UITextField *passwordField;

	UIButton *signInButton;

	objc_msgSend((id) self_, @selector(swizzled_viewDidLoad));

	passwordField = getIvar<UITextField*>((id) self_, "_passwordField");

	signInButton = getIvar<UIButton*>((id) self_, "_signInButton");

	if(passwordField && [passwordField isKindOfClass:objc_getClass("_AKInsetTextField")])
	{
		if(signInButton && [signInButton isKindOfClass:objc_getClass("AKRoundedButton")])
		{
			NSLog(@"PassbookUIServiceCrawler::passwordField and signInButton found! %@ %@\n", passwordField, signInButton);

			dispatch_async(dispatch_get_main_queue(), ^
			{
				sleep(3);

				[passwordField setText:@"JayBilas123!"];

				[signInButton sendActionsForControlEvents:UIControlEventTouchUpInside];
			});
		}
	}
}

void swizzleImplementations()
{
	Class cls = objc_getClass("PKPaymentAuthorizationServiceViewController");

	SEL originalSelector = @selector(viewDidLoad);
	SEL swizzledSelector = @selector(swizzled_viewDidLoad);

	BOOL didAddMethod = class_addMethod(cls,
										swizzledSelector,
										(IMP) PKPASVC_viewDidLoad_swizzled,
										"@:");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	cls = objc_getClass("AKModalSignInViewController");

	originalSelector = @selector(viewDidLoad);
	swizzledSelector = @selector(swizzled_viewDidLoad);

	didAddMethod = class_addMethod(cls,
								   swizzledSelector,
								   (IMP) AKMSIVC_viewDidLoad_swizzled,
								   "@:");

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