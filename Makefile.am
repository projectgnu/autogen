##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Tue Mar 16 11:18:50 1999				      
##            by:    Gary V. Vaughan <garyv@oranda.demon.co.uk>		      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.4 1999/03/16 15:54:57 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

include $(top_srcdir)/MakeRules.in

SUBDIRS		= compat config autoopts src getdefs columns tests doc

EXTRA_DIST	= NOTES VERSION TODO configure ChangeLog.1
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		  configure libtool stamp-h.in

configure : VERSION

## Makefile.am ends here
