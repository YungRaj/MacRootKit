#include "Crawler.hpp"

namespace NSDarwin
{

namespace AppCrawler
{

CrawlManager::CrawlManager(UIApplication *application, UIApplicationDelegate *delegate)
{
	this->crawlingTimer = [[NSTimer alloc] init];
}

NSArray* CrawlManager::getEligibleViewsForUserInteraction()
{

}
				
NSArray* CrawlManager::getViewsWithClassName(NSArray *views, const char *class_name)
{

}

void CrawlManager::onViewControllerViewDidLoad(UIViewController *viewController)
{

}

}
}