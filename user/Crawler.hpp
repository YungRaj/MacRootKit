#ifdef __CRAWLER_HPP_
#define __CRAWLER_HPP_

#include <UIKit/UIKit.h>

#include "ObjC.hpp"
#include "Swift.hpp"

class Crawler
{
	public:
		explicit Crawler(UIApplication *application, UIApplicationDelegate *delegate);

		~Crawler();

		UIApplication* getApplication() { return application; }

		UIApplicationDelegate* getAppDelegate() { return applicationDelegate; }

		UIViewController* getCurrentViewController() { return currentViewController; }

		NSArray* getViews() { return [currentViewController.view subviews]; }

		void setCurrentViewController(UIViewController *viewController) { this->currentViewController = currentViewController; }

		NSArray* getEligibleViewsForUserInteraction();
		
		NSArray* getViewsWithClassName(NSArray *views, const char *class_name);

	private:
		UIApplication *application;

		UIApplicationDelegate *delegate;

		UIViewController *currentViewController;

		NSArray *viewControllers;
		NSArray *views;

		MachO *macho;

		Dyld *dyld;

		ObjCData *objc;

		Swift *swift;
};

#endif