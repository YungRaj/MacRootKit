set -e

clang -g -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk -arch arm64 -target arm64-apple-ios16 -framework UIKit -framework CoreLocation -framework Foundation -shared user/fake_corelocation_location.mm -o build/libfakeLocation.dylib

codesign -fs - --deep --entitlements entitlements.xml ./build/libfakeLocation.dylib