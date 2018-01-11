if HAVE_DOXYGEN
doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	date > doxyfile.stamp

CLEANFILES = doxyfile.stamp
DISTCLEANFILES = Doxyfile Makefile Makefile.in

all-local: doxyfile.stamp

clean-local:
	rm -rf $(top_srcdir)/docs/html \
	       $(top_srcdir)/docs/latex

endif
