set -e

export CFLAGS=""
export LDFLAGS=""

cd keystone

cd build

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64;X86" -G "Unix Makefiles" ..

cd ..

sudo make install

cd ..

make -f make_lib.mk clean

make -f make_lib.mk