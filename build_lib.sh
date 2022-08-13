set -e

export CFLAGS=""
export LDFLAGS=""

cd keystone

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