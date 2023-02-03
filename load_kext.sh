set -e

if [ "$1" == "arm64e" ]; then
	sudo nvram boot-args="debug=0x100 keepsyms=1 amfi_get_out_of_my_way=1 amfi_allow_any_signature=1 --arm64e_preview_abi"
elif [ "$1" == "x86_64" ]; then
	sudo nvram boot-args="debug=0x100 keepsyms=1 amfi_get_out_of_my_way=1 amfi_allow_any_signature=1"
else
	echo Bad Architecture $1

	exit 1
fi

if [ -d build/com.YungRaj.MacRootKit.kext ]; then
	sudo rm -R build/com.YungRaj.MacRootKit.kext
fi

sudo /usr/bin/kmutil install --volume-root /

./build_kext.sh $1

sudo chmod -R 755 build/com.YungRaj.MacRootKit.kext
sudo chown -R root:wheel build/com.YungRaj.MacRootKit.kext

sudo kextload -v build/com.YungRaj.MacRootKit.kext