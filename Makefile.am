##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Tue Sep 15 11:35:46 1998
##            by:    Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.3 1998/09/28 19:32:54 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

SUBDIRS = compat config autoopts src getdefs columns tests doc

EXTRA_DIST	= NOTES VERSION TODO configure ChangeLog.1
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		 configure libtool stamp-h.in

configure : VERSION

## Makefile.am ends here
