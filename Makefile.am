
##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Fri Sep 11 07:06:59 1998
##            by:    Bruce Korb <bkorb@ellen>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.1 1998/09/14 14:33:44 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

SUBDIRS = compat config autoopts src tests doc getdefs

EXTRA_DIST	= NOTES VERSION TODO configure
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		 configure libtool stamp-h.in

configure : VERSION

## Makefile.am ends here
