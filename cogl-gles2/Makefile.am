include $(top_srcdir)/build/autotools/Makefile.am.silent

# preamble

NULL =

lib_LTLIBRARIES = libdeepin-cogl-gles2.la

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir)

if !USE_GLIB
AM_CPPFLAGS += -I$(top_builddir)/deps/glib
endif

AM_CFLAGS = $(COGL_DEP_CFLAGS) $(COGL_EXTRA_CFLAGS) $(MAINTAINER_CFLAGS)

libdeepin_cogl_gles2_la_SOURCES = cogl-gles2-api.c
libdeepin_cogl_gles2_la_LDFLAGS = \
	-no-undefined \
	-version-info @COGL_LT_CURRENT@:@COGL_LT_REVISION@:@COGL_LT_AGE@ \
	-export-dynamic \
	-export-symbols-regex "^gl*"

coglgles2includedir = $(includedir)/deepin-cogl/cogl-gles2/GLES2
coglgles2include_HEADERS = \
	GLES2/gl2.h \
	GLES2/gl2ext.h \
	GLES2/gl2platform.h

pc_files = cogl-gles2-1.0.pc \
	   cogl-gles2-2.0-experimental.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pc_files)

# rename files with prefix 'deepin-'
install-data-hook:
	mv $(DESTDIR)$(pkgconfigdir)/cogl-gles2-1.0.pc $(DESTDIR)$(pkgconfigdir)/deepin-cogl-gles2-1.0.pc
	mv $(DESTDIR)$(pkgconfigdir)/cogl-gles2-2.0-experimental.pc $(DESTDIR)$(pkgconfigdir)/deepin-cogl-gles2-2.0-experimental.pc

