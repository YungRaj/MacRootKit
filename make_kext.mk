BUILD = build
OBJ = obj

CC = clang
CXX = clang++
NASM = nasm

NONE = NONE

PKG = com.YungRaj.MacRootKit
TARGET = MacRootKit

KFWK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework
IOKIT_FWK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/IOKit.framework

COMMON_CSOURCES := $(wildcard mac_rootkit/*.c)
KERNEL_CSOURCES := $(wildcard kernel/*.c)

COMMON_COBJECTS = $(patsubst mac_rootkit/%.c, $(OBJ)/%.o, $(COMMON_CSOURCES))
KERNEL_COBJECTS = $(patsubst kernel/%.c, $(OBJ)/%.o, $(KERNEL_CSOURCES))

COMMON_CPPSOURCES := $(wildcard mac_rootkit/*.cpp)
KERNEL_CPPSOURCES := $(wildcard kernel/*.cpp)

X86_64_CPPSOURCES := $(wildcard x86_64/*.cpp)
ARM64_CPPSOURCES := $(wildcard arm64/*.cpp)

COMMON_CPPOBJECTS := $(patsubst mac_rootkit/%.cpp, $(OBJ)/%.o, $(COMMON_CPPSOURCES))
KERNEL_CPPOBJECTS := $(patsubst kernel/%.cpp, $(OBJ)/%.o, $(KERNEL_CPPSOURCES))

X86_64_CPPOBJECTS := $(patsubst x86_64/%.cpp, $(OBJ)/%.o, $(X86_64_CPPSOURCES))
ARM64_CPPOBJECTS := $(patsubst arm64/%.cpp, $(OBJ)/%.o, $(ARM64_CPPSOURCES))

X86_64_ASMSOURCES := $(wildcard x86_64/*.s)
X86_64_ASMOBJECTS := $(patsubst x86_64/%.s, $(OBJ)/%.o, $(X86_64_ASMSOURCES))

KERNEL_HEADERS = -I$(KFWK)/Headers -I$(IOKIT_FWK)/Headers

CFLAGS ?=-g -Wno-shadow -Wno-visibility -Wno-unused-variable -O2 -g -fno-builtin -fno-common -nostdinc -mkernel -D_KERNEL_ -DMACH_KERNEL_PRIVATE -DCAPSTONE_HAS_X86=1 -DCAPSTONE_HAS_ARM64=1 -DCAPSTONE_HAS_OSXKERNEL=1 -I./capstone/include -I./kernel -I./mac_rootkit -I./ -I./include -nostdlib

LFLAGS ?= -g /usr/local/lib/libcapstone.a -std=c++11  -Wc++11-extensions -nostdlib -D_KERNEL_ -DMACH_KERNEL_PRIVATE -DCAPSTONE_HAS_X86=1 -DCAPSTONE_HAS_ARM64=1 -DCAPSTONE_HAS_OSXKERNEL=1 -I./capstone/include -I./kernel -I./mac_rootkit -I./ -Iinclude -Wl,-kext -lkmod -lkmodc++ -lcc_kext

CXX_FLAGS = -g -D_KERNEL_ -DMACH_KERNEL_PRIVATE -Wno-inconsistent-missing-override -Wno-unused-variable -std=c++11 -Wc++11-extensions -Wno-sign-conversion -Wno-writable-strings

ASM_FLAGS = -f macho64

.PHONY: all clean

all: $(OBJ) $(BUILD)/$(PKG).kext/Contents/MacOS $(BUILD)/$(PKG).kext/Contents/MacOS/$(TARGET) $(BUILD)/$(PKG).kext/Contents/Info.plist

$(COMMON_COBJECTS): $(OBJ)/%.o: mac_rootkit/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_COBJECTS): $(OBJ)/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_CPPOBJECTS): $(OBJ)/%.o: mac_rootkit/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(KERNEL_CPPOBJECTS): $(OBJ)/%.o: kernel/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(X86_64_CPPOBJECTS): $(OBJ)/%.o: x86_64/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(ARM64_CPPOBJECTS): $(OBJ)/%.o: arm64/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -g -c $< -o $@

$(X86_64_ASMOBJECTS): $(OBJ)/%.o: x86_64/*.s
	$(NASM) $(ASM_FLAGS) $< -o $@

$(OBJ):
	rm $(OBJ)/*.o

$(BUILD)/$(PKG).kext/Contents/MacOS:
	mkdir -p $@
	touch $@/$(TARGET)

$(BUILD)/$(PKG).kext/Contents/MacOS/$(TARGET):  $(COMMON_COBJECTS) $(COMMON_CPPOBJECTS) $(KERNEL_COBJECTS) $(KERNEL_CPPOBJECTS) $(ARM64_CPPOBJECTS) $(X86_64_CPPOBJECTS) $(X86_64_ASMOBJECTS)
	$(CXX) $(LFLAGS) -framework IOKit -o $@ $(KERNEL_COBJECTS) $(COMMON_COBJECTS) $(COMMON_CPPOBJECTS) $(KERNEL_CPPOBJECTS) $(ARM64_CPPOBJECTS) $(X86_64_CPPOBJECTS) $(X86_64_ASMOBJECTS)

$(BUILD)/$(PKG).kext/Contents/Info.plist: Info.plist | $(BUILD)/$(PKG).kext/Contents/MacOS
	cp -f $< $@

clean:
	rm -rf obj/*
	rm -rf $(BUILD)/$(PKG).kext