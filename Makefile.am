SUBDIRS = src .

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sangster_atmega328p-0.1.pc


.PHONY: bootstrap
bootstrap:
	./bootstrap.sh
	./configure --host=avr
	make
