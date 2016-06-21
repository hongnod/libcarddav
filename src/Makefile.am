AUTOMAKE_OPTIONS = gnu

INCLUDES = @CURL_CFLAGS@ @GLIB_CFLAGS@ \
		   -I$(top_srcdir) -I$(top_builddir)

if STATIC_LINK
noinst_LTLIBRARIES = libcarddav.la
endif

if DYNAMIC_LINK
lib_LTLIBRARIES = libcarddav.la
endif

libcarddav_la_LDFLAGS = -version-info @LIBVERSION@

libcarddav_la_SOURCES = \
			carddav.h \
			carddav.c \
			add-carddav-object.c \
			add-carddav-object.h \
			delete-carddav-object.c \
			delete-carddav-object.h \
			modify-carddav-object.c \
			modify-carddav-object.h \
			get-carddav-report.c \
			get-carddav-report.h \
			get-display-name.c \
			get-display-name.h \
			carddav-utils.c \
			carddav-utils.h \
			md5.c \
			md5.h \
			options-carddav-server.c \
			options-carddav-server.h \
			lock-carddav-object.c \
			lock-carddav-object.h

libcarddav_includedir=$(includedir)/libcarddav
libcarddav_include_HEADERS = carddav.h

noinst_HEADERS = \
			add-carddav-object.h \
			delete-carddav-object.h \
			modify-carddav-object.h \
			get-carddav-report.h \
			carddav-utils.h

libcarddav_la_LIBADD = \
			@CURL_LIBS@ \
			@GLIB_LIBS@

