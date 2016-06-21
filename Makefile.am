AUTOMAKE_OPTIONS = gnu

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

if BUILD_DOC
include aminclude.am

MOSTLYCLEANFILES = \
				   $(DX_CLEANFILES) \
				   doc

doxydir = @DX_DOCDIR@
dist_man3_MANS = $(doxydir)/man/man3/carddav.h.3
$(dist_man3_MANS): doxygen-doc

libcarddav_apidoc_filedir = $(docdir)
libcarddav_apidoc_file_DATA = \
						  doc/libcarddav.pdf
endif

libcarddav_configdir = $(libdir)/pkgconfig
libcarddav_config_DATA = libcarddav.pc

EXTRA_DIST = \
			 README \
			 COPYING \
			 AUTHORS \
			 ChangeLog \
			 INSTALL \
			 NEWS \
			 TODO \
			 rfc4791.pdf \
			 version.sh \
			 $(dx_config) \
			 doxygen-api.cfg \
			 libcarddav.pc \
			 autogen.sh
