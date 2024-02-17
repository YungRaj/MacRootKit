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
#include <ptrauth.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <IOKit/IOKitLib.h>

#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>

#include <vector>

template<typename classname>
static inline classname getIvar(id self, const char *name)
{
	Ivar ivar(class_getInstanceVariable(object_getClass(self), name));
	
	void *pointer(ivar == NULL ? NULL : reinterpret_cast<char *>(self) + ivar_getOffset(ivar));
	
	return *reinterpret_cast<classname *>(pointer);
}

@interface ADSPurchaseManger

@end

@interface ADSPurchase

@end

@interface ASKRestrictions

-(bool)isAppInstallationAllowed;

@end

@interface _TtC19AppStoreKitInternal23AppOfferButtonPresenter

-(void)offerButtonTapped;

@end

@interface _TtC9App_Store15OfferButtonView

-(_TtC19AppStoreKitInternal23AppOfferButtonPresenter*)target;

-(void)setAccessibilityOfferIsImmediateBuy:(bool)value;

-(void)performClick:(id)click;

@end

@interface _TtC9App_Store17ProductLockupView

-(id)initWithFrame:(CGRect)frame;

-(NSArray*)accessibilityChildren;

@end

@interface ASDAppQuery
{
	NSDictionary *_resultCache;
}

+(ASDAppQuery*)queryForBundleIDs:(NSString*)bundleID;
+(ASDAppQuery*)queryForStoreApps;
+(ASDAppQuery*)queryForStoreItemIDs:(NSArray*)storeItemID;
+(ASDAppQuery*)queryWithPredicate:(id)predicate;

-(void)executeQueryWithResultHandler:(void (^)(void))handler;

@end

struct AppStoreCrawlServer;
struct AppStoreCrawlClient;

void* app_store_crawl_start_server(void *server);
void* app_store_crawl_start_client(void *client);

extern "C"
{
	void init_app_store_crawl_server(uint16_t port);
	void init_app_store_crawl_client(int socket);
}

static AppStoreCrawlServer *appStoreCrawlServer;

static std::vector<AppStoreCrawlClient*> appStoreCrawlClients;

_TtC9App_Store15OfferButtonView* CrawlForOfferButton()
{
	NSView *view = [[[NSApplication sharedApplication] windows][0] contentView];

	if([view isKindOfClass:objc_getClass("_TtC9App_Store11BaseTabView")])
	{
		NSLog(@"AppStoreCrawler::_TtC9App_Store11BaseTabView was found!");

		view = [[view subviews][0] subviews][0];

		if([view isKindOfClass:objc_getClass("_TtC9App_Store16BaseTabSplitView")])
		{
			NSArray *views = [view subviews];

			view = NULL;

			for(NSView *v in views)
			{
				if([v isKindOfClass:objc_getClass("_NSSplitViewItemViewWrapper")])
				{
					NSLog(@"AppStoreCrawler::_NSSplitViewItemViewWrapper was found!");

					view = v;

					if([[view subviews][0] isKindOfClass:objc_getClass("_TtC9App_Store14BackgroundView")])
					{
						break;
					}
				}
			}

			if(!view)
			{
				NSLog(@"AppStoreCrawler::could not find split view!\n");

				return NULL;
			}

			view = [view subviews][0];

			if([view isKindOfClass:objc_getClass("_TtC9App_Store14BackgroundView")])
			{
				view = [[view subviews][1] subviews][0];

				NSLog(@"AppStoreCrawler::_TtC9App_Store14BackgroundView was found!");

				if([view isKindOfClass:objc_getClass("_TtC9App_Store16AXFilterableView")])
				{
					view = [view subviews][0];

					NSLog(@"AppStoreCrawler::_TtC9App_Store16AXFilterableView was found!");

					if([view isKindOfClass:objc_getClass("NSScrollView")])
					{
						views = [view subviews];

						NSLog(@"AppStoreCrawler::NSScrollView was found!");

						for(NSView *v in views)
						{
							view = v;

							if([view isKindOfClass:objc_getClass("_TtC9App_Store18NavigationClipView")])
							{
								NSLog(@"AppStoreCrawler::_TtC9App_Store18NavigationClipView was found!");

								NSArray *vviews = [view subviews];

								view = NULL; 

								for(NSView *vv in vviews)
								{
									if([vv isKindOfClass:objc_getClass("_TtC9App_Store21NavigationPaletteView")])
									{
										view = vv;

										break;
									}
								}

								if(view)
								{
									view = [[view subviews][0] subviews][0];

									NSLog(@"AppStoreCrawler::_TtC9App_Store21NavigationPaletteView was found!");

									if([view isKindOfClass:objc_getClass("_TtC9App_Store17ProductLockupView")])
									{
										_TtC9App_Store17ProductLockupView *productLockupView = (_TtC9App_Store17ProductLockupView*) view;

										NSLog(@"AppStoreCrawler::_TtC9App_Store17ProductLockupView was found!");

										for(NSView *vv in [productLockupView accessibilityChildren])
										{
											if([vv isKindOfClass:objc_getClass("_TtC9App_Store15OfferButtonView")])
											{
												NSLog(@"AppStoreCrawler::_TtC9App_Store15OfferButtonView was found!");

												return (_TtC9App_Store15OfferButtonView*) vv;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}

NSString* GetBundleID(NSString *appID)
{
	NSNumber *storeItemID;

	NSNumberFormatter *formatter;

	NSString *path;

	_TtC9App_Store15OfferButtonView *offerButtonView = CrawlForOfferButton();

	if(!offerButtonView)
	{
		return NULL;
	}

	formatter = [[NSNumberFormatter alloc] init];

	formatter.numberStyle = NSNumberFormatterDecimalStyle;

	storeItemID = [formatter numberFromString:appID];

	dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

	dispatch_async(dispatch_get_main_queue(), ^{
		// [offerButtonView setAccessibilityOfferIsImmediateBuy:YES];

		object_setInstanceVariable([offerButtonView target], "theme", 0);

		id buttonAction = getIvar<id>([offerButtonView target], "buttonAction");
		id buyAction = getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "buyAction");

		id confirmationAccessibilityAction = getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction");

		id offerAction = getIvar<id>(confirmationAccessibilityAction3, "defaultAction");

		NSLog(@"AppStoreCrawler::appID buttonAction = %s 0x%llx", class_getName(object_getClass(getIvar<id>([offerButtonView target], "buttonAction"))), (uint64_t) getIvar<id>([offerButtonView target], "buttonAction"));
		NSLog(@"AppStoreCrawler::appID \t->buyAction = %s 0x%llx", class_getName(object_getClass(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "buyAction"))), (uint64_t) getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "buyAction"));
		NSLog(@"AppStoreCrawler::appID confirmationInitiationAction = %s", class_getName(object_getClass(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationInitiationAction"))));
		NSLog(@"AppStoreCrawler::appID confirmationAccessibilityAction = %s", class_getName(object_getClass(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction"))));
		NSLog(@"AppStoreCrawler::appID \t->defaultAction = %s 0x%llx", class_getName(object_getClass(getIvar<id>(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction"), "defaultAction"))), (uint64_t) getIvar<id>(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction"), "defaultAction"));
		NSLog(@"AppStoreCrawler::appID \t->buyAction = %s 0x%llx", class_getName(object_getClass(getIvar<id>(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction"), "buyAction"))), (uint64_t) getIvar<id>(getIvar<id>(getIvar<id>([offerButtonView target], "buttonAction"), "confirmationAccessibilityAction"), "buyAction"));
		NSLog(@"AppStoreCrawler::appID restrictions = %@", getIvar<id>([offerButtonView target], "restrictions"));
		NSLog(@"AppStoreCrawler::appID \t>isAppInstallationAllowed = %d", [getIvar<ASKRestrictions*>([offerButtonView target], "restrictions") isAppInstallationAllowed]);
		NSLog(@"AppStoreCrawler::appID stateMachine = %s", class_getName(object_getClass(getIvar<id>([offerButtonView target], "stateMachine"))));
		NSLog(@"AppStoreCrawler::appID \t->state = 0x%llx", (uint64_t) getIvar<id>(getIvar<id>([offerButtonView target], "stateMachine"), "state"));
		NSLog(@"AppStoreCrawler::appID currentState = 0x%llx", (uint64_t) getIvar<id>([offerButtonView target], "currentState"));
		NSLog(@"AppStoreCrawler::appID previousState = 0x%llx", (uint64_t) getIvar<id>([offerButtonView target], "previousState"));

		NSLog(@"AppStoreCrawler::bundle ID = %@ 0x%llx\n", getIvar<NSString*>(offerAction, "bundleId"), getIvar<NSString*>(offerAction, "bundleId"));
		/*
		[[offerButtonView target] offerButtonTapped];

		sleep(2);

		[[offerButtonView target] offerButtonTapped];

		dispatch_semaphore_signal(semaphore);
		*/
	});

	return NULL;
}

int OpenAppStoreURL(NSString *appID)
{
	NSURL *URL;

	NSString *link;

	NSString *macAppStore = @"macappstore://apps.apple.com/us/app/id%@";

	link = [NSString stringWithFormat:macAppStore, appID];

	URL = [NSURL URLWithString:link];

	if(URL)
	{
		[[NSWorkspace sharedWorkspace] openURL:URL];

		return 0;
	}

	return -1;
}

struct AppStoreCrawlServer
{
	pthread_t thread;
	uint16_t port;
	int socket_;

	AppStoreCrawlServer(uint16_t port)
	{
		this->port = port;
		this->socket_ = -1;
	}

	~AppStoreCrawlServer()
	{
		if(socket_ != -1)
		{
			close(socket_);
		}
	}

	void Listen()
	{
		sockaddr_in address;

		int value;

		this->socket_ = socket(PF_INET, SOCK_STREAM, 0);

		setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &(value = 1), sizeof(value));

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		bind(socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address));

		listen(socket_, -1);

		while(1)
		{
			socklen_t length(sizeof(address));

			int sock = accept(socket_, reinterpret_cast<sockaddr*>(&address), &length);

			init_app_store_crawl_client(sock);
		}
	}
};

struct AppStoreCrawlClient
{
	pthread_t thread;

	int socket_;

	AppStoreCrawlClient(int socket_)
	{
		this->socket_ = socket_;
	}

	~AppStoreCrawlClient()
	{
		close(this->socket_);
	}

	void Handle()
	{
		int status;

		char received[2048];

		while(1)
		{
			NSString *URL;

			size_t bytes_received;

			bytes_received = recv(socket_, received, 2048, 0);

			if(bytes_received > 0)
			{
				NSString *appID = [NSString stringWithUTF8String:received];

				printf("%s app ID for crawling!\n", [appID UTF8String]);

				status = OpenAppStoreURL(appID);

				sleep(5);

				if(status == 0)
				{
					NSString *path = GetBundleID(appID);
				}
			}
		}
	}
};

void* app_store_crawl_start_server(void *server)
{
	appStoreCrawlServer = reinterpret_cast<AppStoreCrawlServer*>(server);

	appStoreCrawlServer->Listen();

	delete appStoreCrawlServer;

	return NULL;
}

void* app_store_crawl_start_client(void *client)
{
	AppStoreCrawlClient *appStoreCrawlClient = reinterpret_cast<AppStoreCrawlClient*>(client);

	appStoreCrawlClients.add(appStoreCrawlClient);

	appStoreCrawlClient->Handle();

	appStoreCrawlClients.remove(appStoreCrawlClient);

	delete appStoreCrawlClient;

	return NULL;
}

extern "C"
{
	void init_app_store_crawl_server(uint16_t port)
	{
		int status;

		AppStoreCrawlServer *crawlerServer = new AppStoreCrawlServer(port);

		status = pthread_create(&crawlerServer->thread, NULL, app_store_crawl_start_server, crawlerServer);

		assert(status == 0);

		// only need when running as standalone binary
		// pthread_join(crawlerServer->thread, NULL);  
	}

	void init_app_store_crawl_client(int socket)
	{
		int status;

		AppStoreCrawlClient *crawlerClient = new AppStoreCrawlClient(socket);

		status = pthread_create(&crawlerClient->thread, NULL, app_store_crawl_start_client, crawlerClient);

		assert(status == 0);
	}

	__attribute__((constructor))
	static void initializer()
	{
		printf("[%s] initializer()\n", __FILE__);

		init_app_store_crawl_server(1448);
	}

	__attribute__ ((destructor))
	static void finalizer()
	{
		printf("[%s] finalizer()\n", __FILE__);
	}
}