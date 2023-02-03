set -e

export ARCH="$1"

export CFLAGS="-target $1-apple-macos"
export CXXFLAGS="-target $1-apple-macos"
export LDFLAGS="-target $1-apple-macos"

cd keystone

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64;X86" -G "Unix Makefiles" ./

make -j8

sudo make uninstall

sudo make install 

cd ..

export CAPSTONE_ARCHS="aarch64 x86"

cd capstone

sudo make uninstall

make clean

./make.sh mac-universal

sudo ./make.sh mac-universal install

cd ..

make clean

make -f make_test.mk