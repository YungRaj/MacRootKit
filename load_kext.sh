set -e

sudo nvram boot-args="debug=0x100 keepsyms=1 amfi_get_out_of_my_way=1 amfi_allow_any_signature=1 --arm64e_preview_abi"

export CFLAGS="-target arm64e-apple-macos12.0"
export LDFLAGS="-target arm64e-apple-macos12.0"

sudo rm -R build/com.YungRaj.MacRootKit.kext

sudo /usr/bin/kmutil install --volume-root /

make -f make_kext.mk clean

make -f make_kext.mk

sudo chmod -R 755 build/com.YungRaj.MacRootKit.kext
sudo chown -R root:wheel build/com.YungRaj.MacRootKit.kext

sudo kextload -v build/com.YungRaj.MacRootKit.kext