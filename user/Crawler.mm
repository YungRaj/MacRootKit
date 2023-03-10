#include "Crawler.hpp"

static CrawlManager *crawler = NULL;

static NSDarwinAppCrawler_viewDidLoad(void *self_, SEL cmd_)
{
	objc_msgSend((id) self_, @selector(swizzled_viewDidLoad));

	crawler->setCurrentViewController((UIViewController*) self_);
	
	crawler->onViewControllerViewDidLoad();
}

@interface NSDarwinAppCrawler ()
{
    CrawlManager *_crawlManager;
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
    if(self)
    {
        delete _crawlManager;
    }

    _crawlManager = crawlManager;
}

-(CrawlManager*)crawlManager
{
    return _crawlManager;
}

-(void)crawlingTimerDidFire:(NSTimer*)timer
{
	UIViewController *viewController = self.crawlManager->getCurrentViewController();

	NSDictionary *crawlData = [self.crawlManager->getCrawlData() objectForKey:[NSStringFromClass([viewController class])]];

	NSArray *crawledViews = [crawlData objectForKey:@"crawledViews"];

	NSArray *eligbleViews = self.crawlManager->getViewsForUserInteraction();

	for(UIView *view in eligbleViews)
	{
		NSString *viewID= [NSString stringWithFormat:@"%@-%llu-%llu-%llu-%llu", NSStringFromClass([view class]),
																						view.frame.origin.x, view.frame.origin.y, view.frame.size.width, view.frame.size.height];

		if(![crawledViews containsObject:viewID])
		{
			// ALAppLovinVideoViewController
		}
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
		if([subview isKindOfClass:[UIScrollView class]])
		{
			[views addObjectsFromArray:this->getViewsForUserInteractionFromRootView(subview)]

		} else if(subview.userInteractionEnabled)
		{
			[views addObject:subview]
		} else
		{
			[views addObjectsFromArray:this->getViewsForUserInteractionFromRootView(subview)];
		}
	}

	return views;
}

NSArray* CrawlManager::getViewsWithClassName(NSArray *views, const char *class_name)
{

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