set -e

export ARCH="$1"

export CFLAGS="-target $1-apple-macos"
export CXXFLAGS="-target $1-apple-macos"
export LDFLAGS="-target $1-apple-macos"

if [ "$2" == "all" ]; then

	cd capstone

	export CAPSTONE_ARCHS="x86 aarch64"

	make clean

	./make.sh osx-kernel

	sudo ./make.sh osx-kernel install

	cd ..

fi

if [ ! -d "obj" ]; then
	mkdir obj
fi

make -f make_kext.mk clean

make -f make_kext.mk