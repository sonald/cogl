include $(top_srcdir)/build/autotools/Makefile.am.silent

NULL =

BUILT_SOURCES =

CLEANFILES =
DISTCLEANFILES =

EXTRA_DIST =

# tesselator sources
cogl_tesselator_sources = \
	$(srcdir)/tesselator/dict-list.h 	\
	$(srcdir)/tesselator/dict.c 		\
	$(srcdir)/tesselator/dict.h 		\
	$(srcdir)/tesselator/geom.c 		\
	$(srcdir)/tesselator/geom.h 		\
	$(srcdir)/tesselator/gluos.h 		\
	$(srcdir)/tesselator/memalloc.h 	\
	$(srcdir)/tesselator/mesh.c 		\
	$(srcdir)/tesselator/mesh.h 		\
	$(srcdir)/tesselator/normal.c 		\
	$(srcdir)/tesselator/normal.h 		\
	$(srcdir)/tesselator/priorityq-heap.h 	\
	$(srcdir)/tesselator/priorityq-sort.h 	\
	$(srcdir)/tesselator/priorityq.c 	\
	$(srcdir)/tesselator/priorityq.h 	\
	$(srcdir)/tesselator/render.c 		\
	$(srcdir)/tesselator/render.h 		\
	$(srcdir)/tesselator/sweep.c 		\
	$(srcdir)/tesselator/sweep.h 		\
	$(srcdir)/tesselator/tess.c 		\
	$(srcdir)/tesselator/tess.h 		\
	$(srcdir)/tesselator/tesselator.h 	\
	$(srcdir)/tesselator/tessmono.c 	\
	$(srcdir)/tesselator/tessmono.h 	\
	$(srcdir)/tesselator/GL/glu.h 		\
	$(NULL)

source_c = \
	$(cogl_tesselator_sources) \
	cogl-path-private.h \
	cogl1-path.c \
	cogl-path.c \
	$(NULL)

EXTRA_DIST += \
	$(srcdir)/tesselator/README 		\
	$(srcdir)/tesselator/priorityq-heap.c	\
	$(srcdir)/cogl-path.symbols		\
	$(NULL)

source_1_x_h = \
	$(srcdir)/cogl-path-types.h \
	$(srcdir)/cogl1-path-functions.h \
	$(NULL)

source_1_x_intro_h = \
	$(source_1_x_h) \
	$(srcdir)/cogl-path.h \
	$(NULL)

source_2_x_intro_h = \
	$(srcdir)/cogl-path-types.h \
	$(srcdir)/cogl2-path-functions.h \
	$(srcdir)/cogl-path.h \
	$(NULL)

source_h = \
	cogl-path.h \
	$(source_1_x_h) \
	cogl2-path-functions.h \
	$(NULL)

if USE_GLIB
# glib-mkenums rules
glib_enum_h = cogl-path-enum-types.h
glib_enum_c = cogl-path-enum-types.c
glib_enum_headers = $(source_1_x_h)
include $(top_srcdir)/build/autotools/Makefile.am.enums
endif

lib_LTLIBRARIES = libcogl-path.la

libcogl_path_la_SOURCES = $(source_c) $(source_h)
nodist_libcogl_path_la_SOURCES = $(BUILT_SOURCES)
libcogl_path_la_CFLAGS = $(COGL_DEP_CFLAGS) $(COGL_EXTRA_CFLAGS) $(MAINTAINER_CFLAGS)
libcogl_path_la_LIBADD = $(top_builddir)/cogl/libcogl.la
libcogl_path_la_LIBADD += $(COGL_DEP_LIBS) $(COGL_EXTRA_LDFLAGS)
libcogl_path_la_LDFLAGS = \
	-export-dynamic \
	-export-symbols-regex "^(cogl|cogl2)_(framebuffer|path|is|clip|[sg]et)_.*" \
	-no-undefined \
	-version-info @COGL_LT_CURRENT@:@COGL_LT_REVISION@:@COGL_LT_AGE@ \
	-rpath $(libdir)

AM_CPPFLAGS = \
	-DCOGL_COMPILATION		\
	-DG_LOG_DOMAIN=\"CoglPath\"	\
	-I$(srcdir)/tesselator		\
	-I$(top_srcdir)/cogl		\
	-I$(top_builddir)/cogl		\
	-I$(top_srcdir)/cogl/winsys	\
	-I$(top_srcdir)			\
	-I$(top_builddir)

cogl_pathheadersdir = $(includedir)/cogl/cogl-path
cogl_pathheaders_HEADERS = $(source_h)
if USE_GLIB
nodist_cogl_pathheaders_HEADERS = cogl-path-enum-types.h
endif

pc_files = cogl-path-1.0.pc \
	   cogl-path-$(COGL_API_VERSION)-experimental.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pc_files)

EXTRA_DIST += cogl-path.pc.in
DISTCLEANFILES += $(pc_files)

dist-hook:	\
	$(top_builddir)/build/win32/vs9/cogl-path.vcproj	\
	$(top_builddir)/build/win32/vs10/cogl-path.vcxproj	\
	$(top_builddir)/build/win32/vs10/cogl-path.vcxproj.filters

MSVC_PROJECT = cogl-path
MSVC_PROJECT_SRCDIR = cogl-path
MSVC_PROJECT_SRCS = $(source_c)
MSVC_PROJECT_EXCLUDES = dummy

include $(top_srcdir)/build/Makefile-msvcproj.mak

-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =

if HAVE_INTROSPECTION
INTROSPECTION_COMPILER_ARGS=--includedir=$(top_builddir)/cogl

CoglPath-1.0.gir: libcogl-path.la Makefile

CoglPath_1_0_gir_NAMESPACE = CoglPath
CoglPath_1_0_gir_VERSION = 1.0
CoglPath_1_0_gir_LIBS = $(top_builddir)/cogl/libcogl.la libcogl-path.la
CoglPath_1_0_gir_FILES = $(source_1_x_intro_h)
CoglPath_1_0_gir_CFLAGS = $(AM_CPPFLAGS) $(COGL_DEP_CFLAGS) $(COGL_EXTRA_CFLAGS) $(MAINTAINER_CFLAGS)
CoglPath_1_0_gir_INCLUDES =
CoglPath_1_0_gir_EXPORT_PACKAGES = cogl-path-1.0
CoglPath_1_0_gir_SCANNERFLAGS = \
	--warn-all \
	--identifier-prefix=Cogl \
	--symbol-prefix=cogl_ \
	--c-include='cogl-path/cogl-path.h' \
	--include-uninstalled=$(top_builddir)/cogl/Cogl-1.0.gir

CoglPath-2.0.gir: libcogl-path.la Makefile

CoglPath_2_0_gir_NAMESPACE = CoglPath
CoglPath_2_0_gir_VERSION = 2.0
CoglPath_2_0_gir_LIBS = $(top_builddir)/cogl/libcogl.la libcogl-path.la
CoglPath_2_0_gir_FILES = $(source_2_x_intro_h)
CoglPath_2_0_gir_CFLAGS = $(AM_CPPFLAGS) $(COGL_DEP_CFLAGS) $(COGL_EXTRA_CFLAGS) $(MAINTAINER_CFLAGS)
CoglPath_2_0_gir_INCLUDES =
CoglPath_2_0_gir_EXPORT_PACKAGES = cogl-path-2.0-experimental
CoglPath_2_0_gir_SCANNERFLAGS = \
	--warn-all \
	--identifier-prefix=Cogl \
	--symbol-prefix=cogl_ \
	--symbol-prefix=cogl2_ \
	--c-include='cogl-path/cogl-path.h' \
	--include-uninstalled=$(top_builddir)/cogl/Cogl-2.0.gir

INTROSPECTION_GIRS += CoglPath-1.0.gir CoglPath-2.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
