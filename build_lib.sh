set -e

export ARCH="$1"

export CFLAGS="-target $1-apple-macos"
export CXXFLAGS="-target $1-apple-macos"
export LDFLAGS="-target $1-apple-macos"

if [ "$2" == "all" ]; then

    cd capstone

    make clean

    export CAPSTONE_ARCHS="x86 aarch64"

    ./make.sh mac-universal-no

    sudo ./make.sh mac-universal-no install

    cd ..

    cd keystone

    if [ ! -d build ]; then
      mkdir build
    fi

    cd build

    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64;X86" -G "Unix Makefiles" ..

    make -j8

    sudo make install

    cd ../..

fi

make -f make_lib.mk clean

make -f make_lib.mk