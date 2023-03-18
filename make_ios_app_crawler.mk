ARCH = arm64

BUILD = build
OBJ = obj

SDK = iphoneos

SYSROOT := /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk
SYSROOT := /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk

CLANG := $(shell xcrun --sdk $(SDK) --find clang)
CLANGPP := $(shell xcrun --sdk $(SDK) --find clang++)

CC := $(CLANG) -isysroot $(SYSROOT) -arch $(ARCH)
CXX := $(CLANGPP) -isysroot $(SYSROOT) -arch $(ARCH)

NONE = NONE

TARGET = libiOSAppCrawler.dylib

COMMON_SOURCES := user/Crawler.mm $(wildcard user/FakeTouch/*.mm)
COMMON_OBJECTS := $(patsubst user/%.mm, $(OBJ)/%.o, $(COMMON_SOURCES))

CFLAGS += -g -I$(shell pwd)/mac_rootkit -I$(shell pwd)/user/FakeTouch -arch arm64 -target arm64-apple-ios

LDFLAGS += -g -shared -arch arm64 -target arm64-apple-ios -framework UIKit -framework Foundation -framework IOKit -framework CoreGraphics -framework QuartzCore

CXXFLAGS += -g  -I$(shell pwd)/mac_rootkit -arch arm64 -target arm64-apple-macos

.PHONY: all clean

all: $(OBJ) $(BUILD)/$(TARGET)

$(COMMON_OBJECTS): $(OBJ)/%.o: user/%.mm
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJ):
	rm $(OBJ)/*.o

$(BUILD)/$(TARGET):  $(COMMON_OBJECTS)
	$(CXX)  $(LDFLAGS) -o $@ $(COMMON_OBJECTS)

clean:
	rm -rf obj/*
	rm -rf $(BUILD)/$(TARGET)