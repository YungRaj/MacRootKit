set -e

export CFLAGS="-target arm64e-apple-macos12.0"
export LDFLAGS="-target arm64e-apple-macos12.0"

cd build

sudo rm -R com.YungRaj.MacRootKit.kext

cd ..

make -f make_kext.mk clean

make -f make_kext.mk

cd build

sudo chmod -R 755 com.YungRaj.MacRootKit.kext
sudo chown -R root:wheel com.YungRaj.MacRootKit.kext

sudo kextload -v com.YungRaj.MacRootKit.kext

cd ..