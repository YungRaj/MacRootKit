#ifdef __CRAWLER_HPP_
#define __CRAWLER_HPP_

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>

#include "ObjC.hpp"
#include "Swift.hpp"

/* iOS App Crawler for macOS */
/*****************************/
/* Crawler will use depth first search* /
/* Store previously crawled UI elements */

/* NSDictionary
 * {
 *      "viewController" -> NSString
 * 		"className" -> NSString
 *      "frame" -> NSDictionary
 *			"x" -> NSNumber
 *			"y" -> NSNumber
 * 			"width" -> NSNumber
 * 			"height" -> NSNumber
 *		"userInteractionEnabled" -> Bool
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

@interface NSDarwinAppCrawler : NSObject

@property CrawlManager *crawlManager;

@property (assign, nonatomic) NSDictionary *crawlData;

-(instancetype)initWithCrawlingManager:(CrawlManager*)crawlManager;

-(void)crawlingTimerDidFire:(NSTimer*)timer;

@end

namespace NSDarwin
{
	namespace AppCrawler
	{
		class CrawlManager
		{
			public:
				explicit CrawlManager(UIApplication *application, UIApplicationDelegate *delegate);

				~CrawlManager();

				NSDarwinAppCrawler* getCrawler() { return crawler; }

				NSTimer* getCrawlingTimer() { return crawlingTimer; }

				NSString* getBundleID() { return bundleIdentifier; }

				UIApplication* getApplication() { return application; }

				UIApplicationDelegate* getAppDelegate() { return applicationDelegate; }

				UIViewController* getCurrentViewController() { return currentViewController; }

				NSArray* getViews() { return [currentViewController.view subviews]; }

				void setCurrentViewController(UIViewController *viewController) { this->currentViewController = currentViewController; }

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

				void onViewControllerViewDidLoad();

			private:
				NSDarwinAppCrawler *crawler;

				NSString *bundleIdentifier;

				NSTimer *crawlingTimer;

				UIApplication *application;

				UIApplicationDelegate *delegate;

				UIViewController *currentViewController;

				NSArray *viewControllers;
				NSArray *views;
		};
	}
}

#endif