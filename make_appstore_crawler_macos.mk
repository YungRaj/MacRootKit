ARCH = arm64e

BUILD = build
OBJ = obj

SDK = macosx

SYSROOT := $(shell xcrun --sdk $(SDK) --show-sdk-path)
SYSROOT := $(shell xcrun --sdk $(SDK) --show-sdk-path)

CLANG := $(shell xcrun --sdk $(SDK) --find clang)
CLANGPP := $(shell xcrun --sdk $(SDK) --find clang++)

CC := $(CLANG) -isysroot $(SYSROOT) -arch $(ARCH)
CXX := $(CLANGPP) -isysroot $(SYSROOT) -arch $(ARCH)

NONE = NONE

TARGET = libmacOSAppStore_crawler.dylib

COMMON_SOURCES := user/app_store_crawler_macos.mm
COMMON_OBJECTS := $(patsubst user/%.mm, $(OBJ)/%.o, $(COMMON_SOURCES))

CFLAGS += -g -I$(shell pwd)/mac_rootkit -arch arm64e -target arm64e-apple-macos

LDFLAGS += -g -shared -L/usr/local/lib -arch arm64e -target arm64e-apple-macos -framework AppKit -framework Foundation

CXXFLAGS += -g  -I$(shell pwd)/mac_rootkit -arch arm64e -target arm64e-apple-macos

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