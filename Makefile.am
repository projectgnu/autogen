##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <bkorb@gnu.org>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Tue Sep 28 07:17:04 1999				      
##            by:    Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.15 2000/04/04 13:20:47 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS		= compat config snprintfv autoopts \
		agen5 getdefs columns doc

EXTRA_DIST	= VERSION TODO NOTES configure ChangeLog.1
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		  configure libtool stamp-h.in

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
