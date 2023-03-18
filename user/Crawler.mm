#include "Crawler.hpp"

#include "FakeTouch/UIApplication-KIFAdditions.h"
#include "FakeTouch/UIEvent+KIFAdditions.h"
#include "FakeTouch/UITouch-KIFAdditions.h"

#include <assert.h>

using namespace NSDarwin::AppCrawler;

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
	
	crawler->onViewControllerViewDidLoad((UIViewController*) self_);
}

@interface NSViewCrawlData()
{
}

@end

@implementation NSViewCrawlData

@end

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
    [self setCrawlData:[[NSMutableDictionary alloc] init]];

    return self;
}

-(void)setCrawlingManager:(CrawlManager*)crawlManager
{
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

-(NSViewCrawlData*)setupCrawlDataForView:(UIView*)view
{
	NSViewCrawlData *crawlData = [[NSViewCrawlData alloc] init];

	crawlData.name = NSStringFromClass([view class]);
	crawlData.parent = NSStringFromClass([[view superview] class]);

	crawlData.frame = view.frame;
	crawlData.center = view.center;
	crawlData.anchorPoint = view.layer.anchorPoint;

	return crawlData;
}


-(BOOL)hasViewBeenCrawled:(UIView*)view
{
	UIViewController *viewController = self.crawlManager->getCurrentViewController();

	NSDictionary *crawlData = [self.crawlData objectForKey:NSStringFromClass([viewController class])];

	NSMutableDictionary *crawledViews = crawlData ? [crawlData objectForKey:@"crawledViews"] : [[NSMutableDictionary alloc] init];

	if(!crawledViews)
		return false;

	NSMutableArray *viewCrawlData = [crawledViews objectForKey:NSStringFromClass([view class])];

	if(!viewCrawlData)
		return false;

	for(NSUInteger i = 0; i < [viewCrawlData count]; i++)
	{
		NSViewCrawlData *crawlData = [viewCrawlData objectAtIndex:i];

		if([crawlData.name isEqual:NSStringFromClass([view class])] &&
			CGPointEqualToPoint(crawlData.frame.origin, view.frame.origin))
		{
			return true;
		}
	}

	return false;
}

-(void)crawlingTimerDidFire:(NSTimer*)timer
{
	UIViewController *viewController = self.crawlManager->getCurrentViewController();

	NSDictionary *crawlData = [self.crawlData objectForKey:NSStringFromClass([viewController class])];

	NSMutableDictionary *crawledViews = crawlData ? [crawlData objectForKey:@"crawledViews"] : [[NSMutableDictionary alloc] init];

	NSArray *eligbleViews = self.crawlManager->getViewsForUserInteraction();

	if([viewController isKindOfClass:objc_getClass("GADFullScreenAdViewController")])
	{
		if([viewController respondsToSelector:@selector(closeButton)])
		{
			UIButton *closeButton = [viewController performSelector:@selector(closeButton)];

			if(closeButton)
			{
				[closeButton sendActionsForControlEvents:UIControlEventTouchUpInside];

				self.crawlManager->invalidateCrawlingTimer();
				self.crawlManager->setupCrawlingTimer();

				return;
			}
		}
	}

	bool didUserInteraction = false;

	for(UIView *view in eligbleViews)
	{
		NSString *viewClassName = NSStringFromClass([view class]);

		BOOL crawled = [self hasViewBeenCrawled:view];

		if(crawled)
			continue;

		NSMutableArray *viewCrawlData = [crawledViews objectForKey:viewClassName];

		if(!viewCrawlData)
		{
			viewCrawlData = [[NSMutableArray alloc] init];

			[crawledViews setObject:viewCrawlData forKey:viewClassName];
		}

		if(view.userInteractionEnabled)
		{
			if([viewClassName isEqual:@"SKView"])
			{

			}
			
			self.crawlManager->invalidateCrawlingTimer();
			self.crawlManager->setupCrawlingTimer();

			[viewCrawlData addObject:[self setupCrawlDataForView:view]];

			didUserInteraction = true;

			goto done;

		}
	}

done:

	if(!crawlData)
	{
		[self.crawlData setObject:crawledViews forKey:NSStringFromClass([viewController class])];
	}
}

-(void)simulateTouchEventAtPoint:(CGPoint)point
{
	NSMutableArray *touches = [[NSMutableArray alloc] init];

	// Initiating the touch

	UITouch *touch = [[UITouch alloc] initAtPoint:point inWindow:[UIApplication sharedApplication].keyWindow];
    
    [touch setLocationInWindow:point];

    [touches addObject:touch];
    
    UIEvent *event = [[UIApplication sharedApplication] _touchesEvent];

    [event _clearTouches];
    [event kif_setEventWithTouches:touches];
    
    for(UITouch *touch in touches)
    	[event _addTouch:touch forDelayedDelivery:NO];

    [[UIApplication sharedApplication] sendEvent:event];

    [touch setPhaseAndUpdateTimestamp:UITouchPhaseStationary];

    // Ending the touch

    touch = [[UITouch alloc] initTouch];

    [touch setLocationInWindow:point];
    [touch setPhaseAndUpdateTimestamp:UITouchPhaseEnded];

    [touches addObject:touch];

    event = [[UIApplication sharedApplication] _touchesEvent];

    [event _clearTouches];
    [event kif_setEventWithTouches:touches];
    
    for(UITouch *touch in touches)
    	[event _addTouch:touch forDelayedDelivery:NO];

    [[UIApplication sharedApplication] sendEvent:event];
}

@end

namespace NSDarwin
{

namespace AppCrawler
{

CrawlManager::CrawlManager(UIApplication *application, id<UIApplicationDelegate> delegate)
{
	this->application = application;
	this->delegate = delegate;

	this->setupAppCrawler();
}

CrawlManager::~CrawlManager()
{
	[this->crawler release];
}

void CrawlManager::setupAppCrawler()
{
	this->crawler = [[NSDarwinAppCrawler alloc] initWithCrawlingManager:this];

	this->crawlData = this->crawler.crawlData;

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

NSArray* CrawlManager::getViewsForUserInteractionFromRootView(UIView *view)
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
		else if(subview.userInteractionEnabled && [subview window] &&
			   ([subview isKindOfClass:[UIControl class]] ||
			   	[subview isKindOfClass:[UITableViewCell class]] ||
			   	[subview isKindOfClass:[UICollectionViewCell class]]))
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

__attribute__((constructor))
static void initializer()
{
	printf("[%s] initializer()\n", __FILE__);

	crawler = new CrawlManager([UIApplication sharedApplication], [[UIApplication sharedApplication] delegate]);
}

__attribute__ ((destructor))
static void finalizer()
{
	printf("[%s] finalizer()\n", __FILE__);;
}