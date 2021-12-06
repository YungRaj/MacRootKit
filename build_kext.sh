set -e

export CAPSTONE_ARCHS="aarch64 x86"

cd capstone

sudo make uninstall

make clean

./make.sh osx-kernel

sudo ./make.sh osx-kernel install

cd ..

make clean

make -f make_kernel.mk