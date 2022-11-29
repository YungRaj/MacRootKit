set -e

clang -g -arch arm64e -target arm64e-apple-macos -framework AppKit -framework Foundation -shared user/cycript_runner.mm -o build/libcycript_runner.dylib

codesign -fs - --deep --entitlements entitlements.xml ./build/libcycript_runner.dylib
