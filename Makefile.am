##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <bkorb@gnu.org>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 2001/12/29 11:24:55 $
##            by:    Bruce Korb <bkorb@gnu.org>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.2 2001/12/29 11:24:55 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

NOSHELL_DIRS = compat config snprintfv autoopts agen5
SHELL_DIRS   = $(NOSHELL_DIRS) columns getdefs doc
SUBDIRS      = @TOP_DIRLIST@

EXTRA_DIST	= VERSION TODO NOTES configure
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
		  configure libtool stamp-h.in

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
