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

static void NSDarwinAppCrawler_viewDidAppear(void *self_, SEL cmd_, BOOL animated)
{
	objc_msgSend((id) self_, @selector(swizzled_viewDidAppear:), animated);

	crawler->setCurrentViewController((UIViewController*) self_);
	
	crawler->onViewControllerViewDidLoad((UIViewController*) self_);
}

@interface NSMutableArray (Shuffling)

- (void)shuffle;

@end

@implementation NSMutableArray (Shuffling)

-(void)shuffle
{
    NSUInteger count = [self count];

    if (count <= 1)
    	return;

    for (NSUInteger i = 0; i < count - 1; ++i)
    {
        NSInteger remainingCount = count - i;
        NSInteger exchangeIndex = i + arc4random_uniform((u_int32_t )remainingCount);
        
        [self exchangeObjectAtIndex:i withObjectAtIndex:exchangeIndex];
    }
}

@end

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

    [self setCrawlManager:crawlManager];
    [self setCrawlData:[[NSMutableDictionary alloc] init]];

    return self;
}

-(void)setCrawlManager:(CrawlManager*)crawlManager
{
	@synchronized (self)
    {
    	_crawlManager = crawlManager;
    }
}

-(CrawlManager*)crawlManager
{
	@synchronized (self)
    {
        return _crawlManager;
    }

    return _crawlManager;
}

-(void)setCrawlData:(NSMutableDictionary*)crawlData
{
    @synchronized (self)
    {
        _crawlData = crawlData;
    }
}

-(NSMutableDictionary*)crawlData
{
    @synchronized (self)
    {
        return _crawlData;
    }

    return _crawlData;
}

-(NSViewCrawlData*)setupCrawlDataForView:(UIView*)view
{
	NSViewCrawlData *crawlData = [[NSViewCrawlData alloc] init];

	crawlData.name = NSStringFromClass([view class]);
	crawlData.parent = NSStringFromClass([[view superview] class]);

	crawlData.frame = view.frame;
	crawlData.position = [[view superview] convertPoint:view.frame.origin toView:view.window];
	crawlData.center = view.center;
	crawlData.anchorPoint = view.layer.anchorPoint;

	return crawlData;
}


-(BOOL)hasViewBeenCrawled:(UIView*)view inViewController:(UIViewController*)vc
{
	UIViewController *viewController = vc;

	NSDictionary *crawlData = [self.crawlData objectForKey:NSStringFromClass([viewController class])];

	NSMutableDictionary *crawledViews = crawlData ? [crawlData objectForKey:@"crawledViews"] : NULL;

	if(!crawledViews)
		return false;

	NSMutableArray *viewCrawlData = [crawledViews objectForKey:NSStringFromClass([view class])];

	if(!viewCrawlData)
		return false;

	CGPoint pointInWindow = [[view superview] convertPoint:view.frame.origin toView:view.window];

	for(NSUInteger i = 0; i < [viewCrawlData count]; i++)
	{
		NSViewCrawlData *crawlData = [viewCrawlData objectAtIndex:i];

		if([crawlData.name isEqual:NSStringFromClass([view class])] &&
			CGPointEqualToPoint(crawlData.position, pointInWindow))
		{
			return true;
		}
	}

	return false;
}

-(void)crawlingTimerDidFire:(NSTimer*)timer
{
	NSDictionary *userInfo = [timer userInfo];

	UIViewController *viewController = (UIViewController*) userInfo[@"viewController"];

	if([viewController.view window] == NULL || [viewController isKindOfClass:[UINavigationController class]])
		return;

	NSString *vcClassName = NSStringFromClass([viewController class]);

	NSMutableDictionary *crawlData = [self.crawlData objectForKey:vcClassName];

	NSMutableDictionary *crawledViews;

	if(crawlData)
		crawledViews = [crawlData objectForKey:@"crawledViews"];
	else
	{
		crawlData = [[NSMutableDictionary alloc] init];
		crawledViews = [[NSMutableDictionary alloc] init];

		[self.crawlData setObject:crawlData forKey:vcClassName];

		[crawlData setObject:crawledViews forKey:@"crawledViews"];
	}

	NSMutableArray *eligibleViews = self.crawlManager->getViewsForUserInteraction(viewController);

	[eligibleViews shuffle];

	if([viewController isKindOfClass:objc_getClass("GADFullScreenAdViewController")])
	{
		if([viewController respondsToSelector:@selector(closeButton)])
		{
			UIButton *closeButton = [viewController performSelector:@selector(closeButton)];

			if(closeButton)
			{
				[closeButton sendActionsForControlEvents:UIControlEventTouchUpInside];

				return;
			}
		}
	}

	bool didUserInteraction = false;

	for(UIView *view in eligibleViews)
	{
		BOOL crawled = [self hasViewBeenCrawled:view inViewController:viewController];

		if(crawled)
			continue;

		NSString *viewClassName = NSStringFromClass([view class]);

		if(view.userInteractionEnabled)
		{
			if([viewClassName isEqual:@"SKView"])
			{
				continue;
			}

			NSMutableArray *viewCrawlData = [crawledViews objectForKey:viewClassName];

			if(!viewCrawlData)
			{
				viewCrawlData = [[NSMutableArray alloc] init];

				[crawledViews setObject:viewCrawlData forKey:viewClassName];
			}

			CGPoint touchPoint = [[view superview] convertPoint:view.frame.origin toView:view.window];

			[viewCrawlData addObject:[self setupCrawlDataForView:view]];

			[self simulateTouchEventAtPoint:touchPoint];

			didUserInteraction = true;

			goto done;

		}
	}

	if(!didUserInteraction)
	{
		for(UIView *view in eligibleViews)
		{
			NSString *viewClassName = NSStringFromClass([view class]);

			if(view.userInteractionEnabled)
			{
				if([viewClassName isEqual:@"SKView"])
				{
					continue;
				}

				NSMutableArray *viewCrawlData = [crawledViews objectForKey:viewClassName];

				if(!viewCrawlData)
				{
					viewCrawlData = [[NSMutableArray alloc] init];

					[crawledViews setObject:viewCrawlData forKey:viewClassName];
				}

				CGPoint touchPoint = [[view superview] convertPoint:view.frame.origin toView:view.window];

				[self simulateTouchEventAtPoint:touchPoint];

				goto done;

			}
		}
	}

done:
	[timer invalidate];
}

-(void)idlingTimerDidFire:(NSTimer*)timer
{

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

	SEL originalSelector = @selector(viewDidAppear:);
	SEL swizzledSelector = @selector(swizzled_viewDidAppear:);

	BOOL didAddMethod = class_addMethod(UIViewController_class,
										swizzledSelector,
										(IMP) NSDarwinAppCrawler_viewDidAppear,
										"@:");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(UIViewController_class ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(UIViewController_class, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}
}

NSMutableArray* CrawlManager::getViewsForUserInteraction(UIViewController *viewController)
{
	return this->getViewsForUserInteractionFromRootView([viewController view]);
}

NSMutableArray* CrawlManager::getViewsForUserInteractionFromRootView(UIView *view)
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
			   ([subview isKindOfClass:[UIButton class]] ||
			   	[subview isKindOfClass:[UITableViewCell class]] ||
			   	[subview isKindOfClass:[UICollectionViewCell class]]))
		{
			[views addObject:subview];

			[views addObjectsFromArray:this->getViewsForUserInteractionFromRootView(subview)];
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
	this->setupCrawlingTimer(@{@"viewController" : viewController});
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