all: KEXT LIB TEST

KEXT:
	make -f make_kext.mk

LIB:
	make -F make_lib.mk

TEST:
	make -f make_test.mk
