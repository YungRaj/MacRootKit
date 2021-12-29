set -e

cd capstone

export CAPSTONE_ARCHS="x86 aarch64"

export CFLAGS="-target arm64e-apple-macos12.0"
export LDFLAGS="-target arm64e-apple-macos12.0"

make clean

./make.sh osx-kernel

sudo ./make.sh osx-kernel install

cd ..

make -f make_kext.mk clean

make -f make_kext.mk