##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Wed Jul  7 10:46:26 1999				      
##            by:    Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.8 1999/07/07 17:46:39 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

include $(top_srcdir)/MakeRules.in

SUBDIRS		= compat config snprintfv autoopts \
		src getdefs columns tests doc

EXTRA_DIST	= MakeRules.in NOTES VERSION TODO configure ChangeLog.1
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		  configure libtool stamp-h.in

configure : VERSION

## Makefile.am ends here
