BUILD = build
OBJ = obj

CC = clang
CXX = clang++

NONE = NONE

TARGET = libKernelTFP0.dylib

COMMON_CSOURCES := $(wildcard mac_rootkit/*.c)
USER_CSOURCES := $(wildcard user/*.c)

COMMON_COBJECTS = $(patsubst mac_rootkit/%.c, $(OBJ)/%.o, $(COMMON_CSOURCES))
USER_COBJECTS = $(patsubst user/%.c, $(OBJ)/%.o, $(USER_CSOURCES))

COMMON_CPPSOURCES := $(wildcard mac_rootkit/*.cpp)
USER_CPPSOURCES := $(wildcard user/*.cpp)

X86_64_CPPSOURCES := $(wildcard x86_64/*.cpp)
ARM64_CPPSOURCES := $(wildcard arm64/*.cpp)

COMMON_CPPOBJECTS := $(patsubst mac_rootkit/%.cpp, $(OBJ)/%.o, $(COMMON_CPPSOURCES))
USER_CPPOBJECTS := $(patsubst user/%.cpp, $(OBJ)/%.o, $(USER_CPPSOURCES))

X86_64_CPPOBJECTS := $(patsubst x86_64/%.cpp, $(OBJ)/%.o, $(X86_64_CPPSOURCES))
ARM64_CPPOBJECTS := $(patsubst arm64/%.cpp, $(OBJ)/%.o, $(ARM64_CPPSOURCES))

CFLAGS = -Wno-shadow -Wno-unused-variable -g -D_USER_ -DCAPSTONE_HAS_X86=1 -DCAPSTONE_HAS_ARM64=1 -I./keystone/include -I./capstone/include -I./user -I./mac_rootkit -I./

LFLAGS = -framework IOKit -framework CoreFoundation -L/usr/local/lib /opt/homebrew/Cellar/capstone/4.0.2/lib/libcapstone.a /usr/local/lib/libkeystone.a /opt/homebrew/Cellar/keystone/0.9.2/lib/libcapstone.a -std=c++11  -Wc++11-extensions -DCAPSTONE_HAS_X86=1 -DCAPSTONE_HAS_ARM64=1 -I./keystone/include -I./capstone/include -I./user -I./mac_rootkit -I./

CXX_FLAGS = -std=c++11 -Wc++11-extensions -Wno-sign-conversion -Wno-writable-strings

.PHONY: all clean

all: $(OBJ) $(BUILD)/$(TARGET)

$(COMMON_COBJECTS): $(OBJ)/%.o: mac_rootkit/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_COBJECTS): $(OBJ)/%.o: user/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_CPPOBJECTS): $(OBJ)/%.o: mac_rootkit/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(USER_CPPOBJECTS): $(OBJ)/%.o: user/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(X86_64_CPPOBJECTS): $(OBJ)/%.o: x86_64/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -c $< -o $@

$(ARM64_CPPOBJECTS): $(OBJ)/%.o: arm64/%.cpp
	$(CXX) $(CFLAGS) $(CXX_FLAGS) -g -c $< -o $@

$(OBJ):
	rm $(OBJ)/*.o

$(BUILD)/$(TARGET):  $(COMMON_COBJECTS) $(COMMON_CPPOBJECTS) $(USER_COBJECTS) $(USER_CPPOBJECTS) $(ARM64_CPPOBJECTS) $(X86_64_CPPOBJECTS)
	# $(CXX)  $(LFLAGS) -framework CoreFoundation -framework IOKit -o $@ $(COMMON_COBJECTS) $(COMMON_CPPOBJECTS) $(USER_COBJECTS) $(USER_CPPOBJECTS) $(ARM64_CPPOBJECTS) $(X86_64_CPPOBJECTS)
	libtool -o $(BUILD)/libKTFP0.a -static $(COMMON_COBJECTS) $(COMMON_CPPOBJECTS) $(USER_COBJECTS) $(USER_CPPOBJECTS) $(ARM64_CPPOBJECTS) $(X86_64_CPPOBJECTS)

clean:
	rm -rf obj/*
	rm -rf $(BUILD)/$(TARGET)