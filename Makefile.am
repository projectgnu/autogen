##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-07-27 11:42:45 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.9 2002/07/27 19:10:46 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

NOSHELL_DIRS   = compat snprintfv autoopts agen5
SHELL_DIRS     = $(NOSHELL_DIRS) columns getdefs
XML2AG_DIR     = @XML2AG_DIR@ doc
SUBDIRS        = @TOP_DIRLIST@
CFG_DIST       = config/autogen.spec.in config/bootstrap.local
EXTRA_DIST	   = VERSION TODO NOTES configure $(CFG_DIST)
DISTCLEANFILES = .configure.patch stamp-h
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
		  configure libtool stamp-h.in

distcleancheck_listfiles = \
     find -type f -exec 'test -f $(scrdir)/{} || echo {}'

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
