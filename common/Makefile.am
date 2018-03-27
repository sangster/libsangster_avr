SUBDIRS = src docs .

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sangster_atmega328p-1.0.pc


.PHONY: setup_build
setup_build:
	./setup_build.sh


DISTCLEANFILES = Makefile.in aclocal.m4 compile config.guess config.sub \
                 configure depcomp install-sh ltmain.sh missing

distclean-local:
	-rm -rf autom4te.cache
