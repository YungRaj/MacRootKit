set -e

make -f make_ios_app_crawler.mk clean

mkdir obj/FakeTouch

make -f make_ios_app_crawler.mk

codesign -fs - --deep --entitlements entitlements.xml ./build/libiOSAppCrawler.dylib