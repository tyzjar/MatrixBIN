AUTOMAKE_OPTIONS = subdir-objects

MYDEFS = @DEFS@

LIBVERSION = 0:0:0

lib_LTLIBRARIES = $(top_builddir)/lib/libpack.la
__top_builddir__lib_libpack_la_SOURCES = devdrvd_communication.c  crc16.c
__top_builddir__lib_libpack_la_LDFLAGS = -shared -Wc, version-info $(LIBVERSION)

AM_CPPFLAGS = -I$(top_srcdir)/include

__top_builddir__lib_libpack_la_includedir=$(includedir)
__top_builddir__lib_libpack_la_include_HEADERS = ../include/devdrvd_communication.h ../include/crc16.h

AM_LDFLAGS = -static

if DEBUG
AM_CFLAGS = -g3 -O0
else
AM_CFLAGS = -O2
endif

.PHONY: convenience-link clean-convenience-link

convenience-link: $(lib_LTLIBRARIES)
	echo | $(EGREP) "^dlname=" $^ | $(SED) -e "s|^dlname='\(.*\)'|\1|"
	@for soname in `echo | $(EGREP) "^library_names=" $^ | $(SED) -e "s|^library_names='\(.*\)'|\1|"`; do  \
		echo "$$soname: creating convenience link from $(abs_top_builddir)/lib/.libs to $(top_builddir)/lib"; \
		rm -f $(abs_top_builddir)/lib/$$soname ; \
		test -e $(abs_top_builddir)/lib/.libs/$$soname && \
		cd $(abs_top_builddir)/lib/ && \
		$(LN_S) $(abs_top_builddir)/lib/.libs/$$soname $$soname || true;\
	done

clean-convenience-link:
	@for soname in `echo | $(EGREP) "^dlname=" $(lib_LTLIBRARIES) | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
		echo "$$soname: cleaning convenience links"; \
		test -L $(abs_top_builddir)/lib/$$soname && rm -f $(abs_top_builddir)/lib/$$soname || true; \
	done 

all-local:: convenience-link

clean-local:: clean-convenience-link
