##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-07-03 16:43:55 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.7 2002/07/03 23:44:50 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

NOSHELL_DIRS = compat config snprintfv autoopts agen5
SHELL_DIRS   = $(NOSHELL_DIRS) columns getdefs
XML2AG_DIR   = @XML2AG_DIR@ doc
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
