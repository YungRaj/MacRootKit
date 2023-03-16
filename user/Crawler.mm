#include "Crawler.hpp"

static CrawlManager *crawler = NULL;

static bool NSDarwinAppCrawlerClassContainsAdsPrefix(NSString *className)
{
	return [className hasPrefix:@"GAD"] ||
		   [className hasPrefix:@"GAM"];
}

static void NSDarwinAppCrawler_viewDidLoad(void *self_, SEL cmd_)
{
	objc_msgSend((id) self_, @selector(swizzled_viewDidLoad));

	crawler->setCurrentViewController((UIViewController*) self_);
	
	crawler->onViewControllerViewDidLoad();
}

@interface NSDarwinAppCrawler ()
{
    CrawlManager *_crawlManager;
    
    NSMutableDictionary *_crawlData;
}

@end

@implementation NSDarwinAppCrawler

-(instancetype)initWithCrawlingManager:(CrawlManager*)crawlManager
{
	self = [super init];
    
    if(!self)
    	return NULL;

    [self setCrawlingManager:crawlManager];
    [self setCrawlData:[NSDictionary dictionary]];

    return self;
}

-(void)setCrawlingManager:(CrawlManager*)crawlManager
{
    if(self.crawlManager)
    {
        delete _crawlManager;
    }

    _crawlManager = crawlManager;
}

-(CrawlManager*)crawlManager
{
    return _crawlManager;
}

-(NSMutableDictionary*)crawlData
{
	if(!_crawlData)
		_crawlData = [[NSMutableDictionary alloc] init];

	return _crawlData;
}

-(void)crawlingTimerDidFire:(NSTimer*)timer
{
	UIViewController *viewController = self.crawlManager->getCurrentViewController();

	NSDictionary *crawlData = [self.crawlData objectForKey:NSStringFromClass([viewController class])];

	NSMutableArray *crawledViews = crawlData ? [crawlData objectForKey:@"crawledViews"] : [[NSMutableArray alloc] init];

	NSArray *eligbleViews = self.crawlManager->getViewsForUserInteraction();

	if([viewController isKindOfClass:objc_getClass("GADFullScreenAdViewController")])
	{
		if([viewController respondsToSelector:@selector(closeButton)])
		{
			UIButton *closeButton = [viewController performSelector:@selector(closeButton)];

			if(closeButton)
			{
				[closeButton sendActionsForControlEvents:UIControlEventTouchUpInside];

				this->invalidateCrawlingTimer();
				this->setupCrawlingTimer();

				return;
			}
		}
	}

	bool didUserInteraction = false;

	for(UIView *view in eligbleViews)
	{
		NSString *viewID = [NSString stringWithFormat:@"%@-%llu-%llu-%llu-%llu", NSStringFromClass([view class]),
																				  view.frame.origin.x, view.frame.origin.y, view.frame.size.width, view.frame.size.height];

		if(![crawledViews containsObject:viewID])
		{
			if(view.userInteractionEnabled)
			{
				this->invalidateCrawlingTimer();
				this->setupCrawlingTimer();

				[crawledViews addObject:viewID];


				didUserInteraction = true;

				goto done;

			} else
			{
				assert(userInteractionEnabled);
			}
		}
	}

	if(!didUserInteraction)
	{
		for(UIView *view in eligbleViews)
		{
			NSString *viewID = [NSString stringWithFormat:@"%@-%llu-%llu-%llu-%llu", NSStringFromClass([view class]),
																					  view.frame.origin.x, view.frame.origin.y, view.frame.size.width, view.frame.size.height];

			if(view.userInteractionEnabled)
			{
				this->invalidateCrawlingTimer();
				this->setupCrawlingTimer();

				[crawledViews addObject:viewID];


				didUserInteraction = true;

				goto done;
			} else
			{
				assert(userInteractionEnabled);
			}
		}

		// either exit(0) or destroy crawl data because somehow all paths have been traversed
	}

done:

	if(!crawlData)
	{
		[self.crawlData setObject:crawledViews forKey:NSStringFromClass([viewController class])];
	}
}

@end

namespace NSDarwin
{

namespace AppCrawler
{

CrawlManager::CrawlManager(UIApplication *application, UIApplicationDelegate *delegate)
{
	this->setupAppCrawler();
}

CrawlManager::~CrawlManager()
{
	[this->crawler release];
}

void CrawlManager::setupAppCrawler()
{
	this->crawler = [[NSDarwinAppCrawler alloc] initWithCrawlingManager:this];

	this->crawlData = [[NSDictionary alloc] init];

	this->bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];

	this->crawlingTimer = [[NSTimer alloc] init];

	Class UIViewController_class = objc_getClass("UIViewController");

	SEL originalSelector = @selector(viewDidLoad);
	SEL swizzledSelector = @selector(swizzled_viewDidLoad);

	BOOL didAddMethod = class_addMethod(UIViewController_class,
										swizzledSelector,
										(IMP) NSDarwinAppCrawler_viewDidLoad,
										"@:");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(UIViewController_class ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(UIViewController_class, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}
}

NSArray* CrawlManager::getViewsForUserInteraction()
{
	return this->getViewsForUserInteractionFromRootView([this->currentViewController view]);
}

NSArray* getViewsForUserInteractionFromRootView(UIView *view)
{
	NSMutableArray *views = [[NSMutableArray alloc] init];

	for(UIView *subview in [view subviews])
	{
		NSString *className = NSStringFromClass([subview class]);

		if(NSDarwinAppCrawlerClassContainsAdsPrefix(className))
			continue;


		if([view isKindOfClass:objc_getClass("UIScrollView")])
		{
			// A user interaction is best made inside of a UIScrollView
			[views addObjectsFromArray:this->getViewsForUserInteractionFromRootView(subview)];
		} 
		else if(subview.userInteractionEnabled)
		{
			[views addObject:subview];
		} else
		{
			[views addObjectsFromArray:this->getViewsForUserInteractionFromRootView(subview)];
		}
	}

	return views;
}

NSArray* CrawlManager::getViewsWithClassName(NSArray *views, const char *class_name)
{
	NSMutableArray *views_ = [[NSMutableArray alloc] init];

	for(UIView *view in views)
	{
		if([view isKindOfClass:objc_getClass(class_name)])
		{
			[views_ addObject:view];
		}
	}

	return views_;
}

void CrawlManager::onViewControllerViewDidLoad(UIViewController *viewController)
{
	this->invalidateCrawlingTimer();
	this->setupCrawlingTimer();
}

}

}

extern "C"
{
	__attribute__((constructor))
	static void initializer()
	{
		printf("[%s] initializer()\n", __FILE__);

		crawler = new Crawler([UIApplication sharedApplication], [[UIApplication sharedApplication] delegate]);
	}

	__attribute__ ((destructor))
	static void finalizer()
	{
		printf("[%s] finalizer()\n", __FILE__);

		delete crawler;
	}
}