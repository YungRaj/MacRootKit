set -e

make -f make_appstore_crawler_ios.mk

sudo codesign -fs - build/libiOSAppStore_crawler.dylib --deep

scp -P 2222 build/libiOSAppStore_crawler.dylib root@localhost:/var/mobile/Downloads