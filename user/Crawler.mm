#include "Crawler.hpp"

static CrawlManager *crawler = NULL;

static NSDarwinAppCrawler_viewDidLoad(void *self_, SEL cmd_)
{
	objc_msgSend((id) self_, @selector(swizzled_viewDidLoad));

	crawler->setCurrentViewController((UIViewController*) self_);
	
	crawler->onViewControllerViewDidLoad();
}

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
	crawler = this;

	this->crawlingTimer = [[NSTimer alloc] init];

	Class cls = objc_getClass("UIViewController");

	SEL originalSelector = @selector(viewDidLoad);
	SEL swizzledSelector = @selector(swizzled_viewDidLoad);

	BOOL didAddMethod = class_addMethod(cls,
										swizzledSelector,
										(IMP) NSDarwinAppCrawler_viewDidLoad,
										"@:");

	if(didAddMethod)
	{
		Method originalMethod = class_getInstanceMethod(cls ,originalSelector);
		Method swizzledMethod = class_getInstanceMethod(cls, swizzledSelector);

		method_exchangeImplementations(originalMethod, swizzledMethod);
	}
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