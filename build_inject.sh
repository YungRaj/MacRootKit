set -e

export ARCH="$1"
export LIBARCHS="$1"

export SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)

export CLANG=$(xcrun --sdk macosx --find clang)
export CLANGPP=$(xcrun --sdk macosx --find clang++)

export CFLAGS="-target $1-apple-macos -arch $1  --sysroot=$SYSROOT -isysroot $SYSROOT"
export CXXFLAGS="-target $1-apple-macos -arch $1 --sysroot=$SYSROOT -isysroot $SYSROOT" 
export LDFLAGS="-target $1-apple-macos -arch $1 --sysroot=$SYSROOT -isysroot $SYSROOT"

if [ "$2" == "all" ]; then

    cd capstone

    make clean

    export CAPSTONE_ARCHS="x86 aarch64"

    ARCH=$1 ./make.sh mac-universal-no

    sudo ./make.sh mac-universal-no install

    cd ..

    cd keystone

    if [ ! -d build ]; then
      mkdir build
    fi

    cd build

    cmake -DCMAKE_OSX_ARCHITECTURES="$1" -DCMAKE_OSX_SYSROOT="$(SYSROOT)" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64;X86" -G "Unix Makefiles" ..

    make -j8

    sudo make install

    cd ../..

fi

export CFLAGS="-target $1-apple-macos -arch $1  --sysroot=$SYSROOT -isysroot $SYSROOT"
export CXXFLAGS="-target $1-apple-macos -arch $1 --sysroot=$SYSROOT -isysroot $SYSROOT" 
export LDFLAGS="-target $1-apple-macos -arch $1 --sysroot=$SYSROOT -isysroot $SYSROOT"

make -f make_inject.mk clean

make -f make_inject.mk