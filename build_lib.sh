set -e

cd capstone

make clean

export CAPSTONE_ARCHS="x86 aarch64"

./make.sh mac-universal-no

sudo ./make.sh mac-universal-no install

cd ..

cd keystone

export CFLAGS="-target arm64e-apple-macos"
export LDFLAGS="-target arm64e-apple-macos"

if [ ! -d build ]; then
  mkdir build
fi

cd build

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64;X86" -G "Unix Makefiles" ..

make -j8

sudo make install

cd ../..

make -f make_lib.mk clean

make -f make_lib.mk