set -e

clang -g -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -arch arm64 -target arm64-apple-macos -framework AppKit -framework Foundation -shared user/cycript_runner.mm -o build/libcycript_runner_arm64.dylib

clang -g -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -arch arm64e -target arm64e-apple-macos -framework AppKit -framework Foundation -shared user/cycript_runner.mm -o build/libcycript_runner_arm64e.dylib

codesign -fs - --deep --entitlements entitlements.xml ./build/libcycript_runner_arm64.dylib

codesign -fs - --deep --entitlements entitlements.xml ./build/libcycript_runner_arm64.dylib

lipo -create -output build/libcycript_runner.dylib build/libcycript_runner_arm64.dylib build/libcycript_runner_arm64e.dylib
