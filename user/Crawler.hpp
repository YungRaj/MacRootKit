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

namespace mrk
{
	namespace appcrawler
	{
		class CrawlerManager
		{
			public:
				explicit CrawlerManager(UIApplication *application, UIApplicationDelegate *delegate);

				~CrawlerManager();

				NSTimer* getCrawlingTimer() { return crawlingTimer; }

				UIApplication* getApplication() { return application; }

				UIApplicationDelegate* getAppDelegate() { return applicationDelegate; }

				UIViewController* getCurrentViewController() { return currentViewController; }

				NSArray* getViews() { return [currentViewController.view subviews]; }

				void setCurrentViewController(UIViewController *viewController) { this->currentViewController = currentViewController; }

				NSArray* getEligibleViewsForUserInteraction();
				
				NSArray* getViewsWithClassName(NSArray *views, const char *class_name);

			private:
				NSTimer *crawlingTimer;

				UIApplication *application;

				UIApplicationDelegate *delegate;

				UIViewController *currentViewController;

				NSDictionary *crawlData;

				NSArray *viewControllers;
				NSArray *views;

				MachO *macho;

				Dyld *dyld;

				ObjCData *objc;

				Swift *swift;
		};
	}
}

#endif