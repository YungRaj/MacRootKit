#ifndef __CRAWLER_HPP_
#define __CRAWLER_HPP_

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SpriteKit/SpriteKit.h>

/* iOS App Crawler for macOS */
/*****************************/
/* Crawler will use depth first search */
/* Store previously crawled UI elements */

extern NSString *kNSDarwinAppCrawlerCrawledViews;

namespace NSDarwin
{
	namespace AppCrawler
	{
		class CrawlManager;
	}
}

using namespace NSDarwin::AppCrawler;

@interface NSViewCrawlData : NSObject

@property (strong, nonatomic) NSString *name;

@property (strong, nonatomic) NSString *parent;

@property (assign, nonatomic) CGRect frame;
@property (assign, nonatomic) CGPoint position; // position in window
@property (assign, nonatomic) CGPoint center;

@property (assign, nonatomic) CGPoint anchorPoint;

@end

@interface NSDarwinAppCrawler : NSObject

@property (atomic) CrawlManager *crawlManager;

@property (strong, atomic) NSTimer *crawlingTimer;
@property (strong, atomic) NSTimer *idleTimer;

@property (strong, atomic) NSMutableDictionary *crawlData;

@property (strong, atomic) NSMutableArray *viewControllerStack;

@property (assign, atomic) CFAbsoluteTime timeSinceLastUserInteraction;

@property (assign, atomic) BOOL spriteKitCrawlCondition;
@property (assign, atomic) BOOL crawlerTimerDidFire;

-(instancetype)initWithCrawlingManager:(CrawlManager*)crawlManager;

-(NSMutableDictionary*)crawlData;

-(NSViewCrawlData*)setupCrawlDataForView:(UIView*)view;

-(UIViewController*)topViewController;
-(UIViewController*)topViewControllerWithRootViewController:(UIViewController*)rootViewController;

-(BOOL)hasViewBeenCrawled:(UIView*)view inViewController:(UIViewController*)vc;

-(void)crawlingTimerDidFire:(NSTimer*)timer;
-(void)idlingTimerDidFire:(NSTimer*) timer;

-(void)simulateTouchEventAtPoint:(CGPoint)point;
-(void)simulateTouchesOnSpriteKitView:(SKView*)view;

-(bool)bypassInterstitialAds:(UIViewController*)vc;

-(void)pushViewControllerToStack:(UIViewController*)vc;

-(BOOL)simulatedTouchesHasHadNoEffect;

@end

namespace NSDarwin
{
	namespace AppCrawler
	{
		class CrawlManager
		{
			public:
				explicit CrawlManager(UIApplication *application, id<UIApplicationDelegate> delegate);

				~CrawlManager();

				NSDarwinAppCrawler* getCrawler() { return crawler; }

				NSTimer* getCrawlingTimer() { return this->crawler.crawlingTimer; }

				NSTimer* getIdleTimer() { return this->crawler.idleTimer; }

				NSString* getBundleID() { return bundleIdentifier; }

				UIApplication* getApplication() { return application; }

				id<UIApplicationDelegate> getAppDelegate() { return delegate; }

				UIViewController* getCurrentViewController() { return currentViewController; }

				NSArray* getViews() { return [currentViewController.view subviews]; }

				void setCurrentViewController(UIViewController *viewController) { this->currentViewController = viewController; }

				void setupAppCrawler();

				inline void setupCrawlingTimer()
				{
					this->invalidateCrawlingTimer();

					this->crawler.crawlingTimer = [NSTimer scheduledTimerWithTimeInterval:1.25f
				                                                  target:this->crawler
				                                                selector:@selector(crawlingTimerDidFire:)
				                                                userInfo:nil
				                                                 repeats:NO];

				}

				inline void setupIdleTimer()
				{
					this->invalidateIdleTimer();

					this->crawler.idleTimer = [NSTimer scheduledTimerWithTimeInterval:3.5f
							                                                  target:this->crawler
							                                                selector:@selector(idlingTimerDidFire:)
							                                                userInfo:nil
							                                                 repeats:NO];
				}

				void invalidateCrawlingTimer() { if(this->crawler.crawlingTimer && [this->crawler.crawlingTimer isValid]) { [this->crawler.crawlingTimer invalidate]; this->crawler.crawlingTimer = NULL; } }

				void invalidateIdleTimer() { if(this->crawler.idleTimer && [this->crawler.idleTimer isValid]) { [this->crawler.idleTimer invalidate]; this->crawler.idleTimer = NULL; } }

				NSMutableArray* getViewsForUserInteraction(UIViewController *viewController);
				NSMutableArray* getViewsForUserInteractionFromRootView(UIView *view);
				
				NSMutableArray* getViewsWithKindOfClass(NSMutableArray *views, Class cls);

				void onViewControllerViewDidAppear(UIViewController *viewController);

			private:
				NSDarwinAppCrawler *crawler;

				NSDictionary *crawlData;

				NSString *bundleIdentifier;

				UIApplication *application;

				id<UIApplicationDelegate> delegate;

				UIViewController *currentViewController;
		};
	}
}

#endif