set -e

make -f make_passbookui_crawler.mk

sudo codesign -fs - build/libPassbookUIService_crawler.dylib --deep

scp -P 2222 build/libPassbookUIService_crawler.dylib root@localhost:/var/mobile/Downloads