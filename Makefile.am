##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-06-12 18:38:15 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.6 2002/06/15 20:20:21 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

NOSHELL_DIRS = compat config snprintfv autoopts agen5
SHELL_DIRS   = $(NOSHELL_DIRS) columns getdefs doc
XML2AG_DIR   = @XML2AG_DIR@
SUBDIRS      = @TOP_DIRLIST@

EXTRA_DIST	= VERSION TODO NOTES configure
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
		  configure libtool stamp-h.in
DISTCLEANFILES = .configure.patch stamp-h

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
