##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-08-18 17:50:19 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.13 2002/08/23 04:15:09 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

NOSHELL_DIRS   = compat snprintfv autoopts agen5
SHELL_DIRS     = $(NOSHELL_DIRS) columns getdefs
XML2AG_DIR     = @XML2AG_DIR@ doc pkg
SUBDIRS        = @TOP_DIRLIST@
config_dist    = config/missing config/misc.def config/libopts.def
pkgdata_DATA   = config/conftest.tpl config/confmacs.tpl
pkg_dist       = pkg/autogen.spec.in config/bootstrap.local
EXTRA_DIST     = VERSION NOTES $(config_dist) $(pkg_dist) $(pkgdata_DATA)
DISTCLEANFILES = .configure.patch stamp-h
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
          configure libtool stamp-h.in config/ag_macros.m4

distcleancheck_listfiles = \
     find -type f -exec 'test -f $(srcdir)/{} || echo {} ;'

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
