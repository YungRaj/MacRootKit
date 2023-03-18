#ifndef __CRAWLER_HPP_
#define __CRAWLER_HPP_

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>

/* iOS App Crawler for macOS */
/*****************************/
/* Crawler will use depth first search */
/* Store previously crawled UI elements */

/* NSDictionary
 * {
 *      "viewController" -> NSArray
 * 			"UIView-300-200-50-20" -> NSString
 * } 
 */

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
	@property (assign, nonatomic) CGPoint center;

	@property (assign, nonatomic) CGPoint anchorPoint;
@end

@interface NSDarwinAppCrawler : NSObject

@property (nonatomic) CrawlManager *crawlManager;

@property (strong, nonatomic) NSMutableDictionary *crawlData;

-(instancetype)initWithCrawlingManager:(CrawlManager*)crawlManager;

-(NSMutableDictionary*)crawlData;

-(NSViewCrawlData*)setupCrawlDataForView:(UIView*)view;

-(BOOL)hasViewBeenCrawled:(UIView*)view;

-(void)crawlingTimerDidFire:(NSTimer*)timer;

-(void)simulateTouchEventAtPoint:(CGPoint)point;

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

				NSTimer* getCrawlingTimer() { return crawlingTimer; }

				NSString* getBundleID() { return bundleIdentifier; }

				UIApplication* getApplication() { return application; }

				id<UIApplicationDelegate> getAppDelegate() { return delegate; }

				UIViewController* getCurrentViewController() { return currentViewController; }

				NSArray* getViews() { return [currentViewController.view subviews]; }

				void setCurrentViewController(UIViewController *viewController) { this->currentViewController = viewController; }

				void setupAppCrawler();

				void setupCrawlingTimer() { [NSTimer scheduledTimerWithTimeInterval:1.5f
							                                                  target:this->crawler
							                                                selector:@selector(crawlingTimerDidFire:)
							                                                userInfo:nil
							                                                 repeats:YES]; }

				void invalidateCrawlingTimer() { if([crawlingTimer isValid]) [crawlingTimer invalidate]; }

				NSArray* getViewsForUserInteraction();
				NSArray* getViewsForUserInteractionFromRootView(UIView *view);
				
				NSArray* getViewsWithClassName(NSArray *views, const char *class_name);

				void onViewControllerViewDidLoad(UIViewController *viewController);

			private:
				NSDarwinAppCrawler *crawler;

				NSDictionary *crawlData;

				NSString *bundleIdentifier;

				NSTimer *crawlingTimer;

				UIApplication *application;

				id<UIApplicationDelegate> delegate;

				UIViewController *currentViewController;

				NSArray *viewControllers;
				NSArray *views;
		};
	}
}

#endif