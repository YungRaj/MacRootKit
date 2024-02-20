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

#include <vector>

template<typename classname>
static inline classname getIvar(id self, const char *name)
{
	Ivar ivar(class_getInstanceVariable(object_getClass(self), name));
	
	void *pointer(ivar == NULL ? NULL : reinterpret_cast<char *>(self) + ivar_getOffset(ivar));
	
	return *reinterpret_cast<classname *>(pointer);
}

@interface _TtC8AppStore11OfferButton : UIControl

@end

@interface _TtC8AppStore31ProductLockupCollectionViewCell : UICollectionViewCell

@end

@interface ASDApp

-(bool)isKindOfClass:(Class)cls;

-(NSString*)artistName;
-(NSString*)localizedName;

-(uint32_t)storeItemID;
-(NSNumber*)storeFront;
-(NSString*)storeCohort;

-(NSString*)bundleID;
-(NSString*)bundlePath;
-(NSString*)bundleVersion;

@end

@interface ASDAppQuery

+(ASDAppQuery*)queryForBundleIDs:(NSString*)bundleID;
+(ASDAppQuery*)queryForStoreApps;
+(ASDAppQuery*)queryForStoreItemIDs:(NSArray*)storeItemID;
+(ASDAppQuery*)queryWithPredicate:(id)predicate;

-(void)executeQueryWithResultHandler:(void (^)(NSArray*))handler;

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

void DownloadApp(NSString *appID);
int OpenAppStoreURL(NSString *appID);

static AppStoreCrawlServer *appStoreCrawlServer;

static std::vector<AppStoreCrawlClient*> appStoreCrawlClients;

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

	bool done;

	NSString *appID;
	NSNumber *storeItemID;

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

		this->done = false;

		while(!this->done)
		{
			NSString *URL;

			size_t bytes_received;

			bytes_received = recv(socket_, received, 2048, 0);

			if(bytes_received > 0)
			{
				NSNumberFormatter *formatter;

				appID = [NSString stringWithUTF8String:received];

				formatter = [[NSNumberFormatter alloc] init];

				formatter.numberStyle = NSNumberFormatterDecimalStyle;

				storeItemID = [formatter numberFromString:appID];

				NSLog(@"%@ app ID for crawling!\n", appID);

				status = OpenAppStoreURL(appID);

				sleep(5);

				if(status == 0)
				{
					DownloadApp(appID);
				}
			} else
			{
				return;
			}
		}

		printf("Finishing client connection!\n");
	}

	void Finish(char *response)
	{
		NSString *path = [NSString stringWithUTF8String:response];

		NSLog(@"AppStore:path = %@", path);

		if(!this->done)
		{
			send(socket_, [path UTF8String], strlen([path UTF8String]), 0);

			this->done = true;

			shutdown(socket_, SHUT_RDWR);
		}
	}
};

_TtC8AppStore11OfferButton* CrawlForOfferButton()
{
	UIView *view = [[[[UIApplication sharedApplication] delegate] window] rootViewController].view;

	if([view isKindOfClass:objc_getClass("UILayoutContainerView")])
	{
		NSLog(@"AppStoreCrawler::UILayoutContainerView was found!");

		view = [[[[[[view subviews][0] subviews][0] subviews][0] subviews][0] subviews][0] subviews][0];

		if([view isKindOfClass:objc_getClass("_TtC8AppStoreP33_F9B20E6387F6F627D5761E6B0A83FE5540InsetCollectionViewControllerContentView")])
		{
			NSLog(@"AppStoreCrawler::_TtC8AppStoreP33_F9B20E6387F6F627D5761E6B0A83FE5540InsetCollectionViewControllerContentView was found!");

			NSArray *subviews = [[view subviews][0] subviews];

			_TtC8AppStore31ProductLockupCollectionViewCell *productLockupViewCell = NULL;

			for(UIView *subview in subviews)
			{
				if([subview isKindOfClass:objc_getClass("_TtC8AppStore31ProductLockupCollectionViewCell")])
				{
					NSLog(@"_TtC8AppStore31ProductLockupCollectionViewCell was found!");

					productLockupViewCell = reinterpret_cast<_TtC8AppStore31ProductLockupCollectionViewCell*>(subview);
				}
			}

			if(productLockupViewCell)
			{
				view = [productLockupViewCell subviews][0];

				subviews = [view subviews];

				for(UIView *subview in subviews)
				{
					if([subview isKindOfClass:objc_getClass("_TtC8AppStore11OfferButton")])
					{
						NSLog(@"AppStoreCrawler:_TtC8AppStore11OfferButton was found!");

						return reinterpret_cast<_TtC8AppStore11OfferButton*>(subview);
					}
				}
			}
		}
	}

	return NULL;
}

bool ClickProductViewControllerBackButton(_TtC8AppStore11OfferButton *offerButton)
{
	UIView *view;

	if(!offerButton)
		offerButton = CrawlForOfferButton();
	
	view = reinterpret_cast<UIView*>(offerButton);

	while(view && ![view isKindOfClass:objc_getClass("UILayoutContainerView")])
	{
		view = [view superview];
	}

	if(!view)
		return false;

	NSLog(@"AppStoreCrawler:UILayoutContainerView was found!");

	for(UIView *subview in [view subviews])
	{
		if([subview isKindOfClass:objc_getClass("UINavigationBar")])
		{
			view = subview;

			break;
		}
	}

	if(!view)
		return false;

	NSLog(@"AppStoreCrawler:UINavigationBar was found!");

	for(UIView *subview in [view subviews])
	{
		if([subview isKindOfClass:objc_getClass("_UINavigationBarContentView")])
		{
			view = subview;

			break;
		}
	}

	if(!view)
		return false;

	NSLog(@"AppStoreCrawler:_UINavigationBarContentView was found!");

	for(UIView *subview in [view subviews])
	{
		if([subview isKindOfClass:objc_getClass("_UIButtonBarButton")])
		{
			view = subview;

			break;
		}
	}

	if(!view)
		return false;

	NSLog(@"AppStoreCrawler:_UIButtonBarButton was found!");

	UIButton *button = reinterpret_cast<UIButton*>(view);

	[button sendActionsForControlEvents:UIControlEventTouchUpInside];

	return true;
}

void FetchInstalledApp()
{
	ASDAppQuery *query = [objc_getClass("ASDAppQuery") queryForStoreApps];

	if(query)
	{
		[query executeQueryWithResultHandler: ^void (NSArray *res)
		{
			ASDApp *installedApp;

			NSArray *results = res;

			NSString *path;

			NSLog(@"AppStore:ASDAppQuery results = %@", results);

			installedApp = NULL;

			for(int i = appStoreCrawlClients.getSize() - 1; i >= 0; i--)
			{
				AppStoreCrawlClient *client = appStoreCrawlClients.get(i);

				NSNumber *storeItemID = client->storeItemID;

				NSLog(@"AppStore::client index %d", i);

				for(int j = 0; j < [results count]; j++)
				{
					ASDApp *app = (ASDApp*) [results objectAtIndex:j];

					if(![app isKindOfClass:objc_getClass("ASDApp")])
						continue;

					if([app bundlePath] && [[app bundlePath] hasPrefix:@"/private/var/containers"])
					{
						if([app storeItemID] == [storeItemID unsignedIntValue])
						{
							installedApp = app;
						}
					}
				}

				NSLog(@"AppStore:installed ASDApp = %@", installedApp);

				if(installedApp)
				{
					NSDictionary *response = @{@"localizedName" : [installedApp localizedName],
										  @"storeItemID" : [NSNumber numberWithUnsignedInt:[installedApp storeItemID]],
										  @"storeFront"  : [installedApp storeFront], 
										  @"storeCohort" : [installedApp storeCohort],
										  @"bundleID"    : [installedApp bundleID],
										  @"bundlePath"  : [installedApp bundlePath],
										  @"bundleVersion" : [installedApp bundleVersion]};

					NSLog(@"AppStore:localizedName = %@ storeCohort = %@ bundleID = %@ bundlePath = %@", [installedApp localizedName], [installedApp storeCohort], [installedApp bundleID], [installedApp bundlePath]);

					NSError *error;

					NSString *fileName = [NSString stringWithFormat:@"%@.plist", [installedApp bundleID]];

					NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

					NSString *documentsDirectory = [paths objectAtIndex:0];

					NSString *plistPath = [documentsDirectory stringByAppendingPathComponent:fileName];

					[response writeToFile:plistPath atomically: YES];

					path = plistPath;

					client->Finish((char*) [path UTF8String]);
				}
			}
		}];
	}
}

void DownloadApp(NSString *appID)
{
	NSNumber *storeItemID;

	NSNumberFormatter *formatter;

	_TtC8AppStore11OfferButton *offerButton = CrawlForOfferButton();

	if(!offerButton)
		return;

	formatter = [[NSNumberFormatter alloc] init];

	formatter.numberStyle = NSNumberFormatterDecimalStyle;

	storeItemID = [formatter numberFromString:appID];

	dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

	dispatch_async(dispatch_get_main_queue(), ^{

		[offerButton sendActionsForControlEvents:UIControlEventTouchUpInside];

		sleep(2);

		ClickProductViewControllerBackButton(offerButton);

		dispatch_semaphore_signal(semaphore);

	});

	dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
}

int OpenAppStoreURL(NSString *appID)
{
	NSURL *URL;

	NSString *link;

	NSString *macAppStore = @"https://apps.apple.com/us/app/id%@";

	link = [NSString stringWithFormat:macAppStore, appID];

	URL = [NSURL URLWithString:link];

	if(URL)
	{
		[[UIApplication sharedApplication] openURL:URL options:@{} completionHandler:nil];

		return 0;
	}

	return -1;
}



void* _newProgressForApp_fromRemoteProgress_usingServiceBroker_swizzled(void *self_, SEL cmd_, void* arg1, void *arg2, void *arg3)
{
	void *orig = (void*) objc_msgSend((id) self_, @selector(_swizzled_newProgressForApp:fromRemoteProgress:usingServiceBroker:), self_, cmd_, arg1, arg2, arg3);

	FetchInstalledApp();

	return orig;
}

void applicationDidEnterBackground_swizzled(void *self_, SEL cmd_, UIApplication *application)
{
	(void) objc_msgSend((id) self_, @selector(swizzled_applicationDidEnterBackground:));

	for(int i = appStoreCrawlClients.getSize() - 1; i >= 0 ; i--)
	{
		AppStoreCrawlClient *client = appStoreCrawlClients.get(i);

		client->Finish((char*) "Error");
	}
}

void swizzleImplementations()
{
	Class cls = objc_getClass("ASDAppQuery");
	Class metacls = objc_getMetaClass("ASDAppQuery");

	SEL originalSelector = @selector(_newProgressForApp:fromRemoteProgress:usingServiceBroker:);
	SEL swizzledSelector = @selector(_swizzled_newProgressForApp:fromRemoteProgress:usingServiceBroker:);

	BOOL didAddMethod = class_addMethod(metacls,
										swizzledSelector,
										(IMP)  _newProgressForApp_fromRemoteProgress_usingServiceBroker_swizzled,
										"@:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getClassMethod(cls ,originalSelector);
		Method swizzledMethod = class_getClassMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}

	cls = objc_getClass("_TtC8AppStore11AppDelegate");

	originalSelector = @selector(applicationDidEnterBackground:);
	swizzledSelector = @selector(swizzled_applicationDidEnterBackground:);

	didAddMethod = class_addMethod(cls,
									swizzledSelector,
									(IMP)  applicationDidEnterBackground_swizzled,
									"@:@");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}
}

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

	printf("Finishing client\n");

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

		signal(SIGPIPE, SIG_IGN);

		init_app_store_crawl_server(1448);

		swizzleImplementations();
	}

	__attribute__ ((destructor))
	static void finalizer()
	{
		printf("[%s] finalizer()\n", __FILE__);
	}
}