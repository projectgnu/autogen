##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    Mon Aug  9 10:10:35 1999				      
##            by:    Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.10 1999/08/09 17:10:52 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

include $(top_srcdir)/MakeRules.in

SUBDIRS		= compat config snprintfv autoopts \
		src getdefs columns tests doc

EXTRA_DIST	= MakeRules.in VERSION TODO configure ChangeLog.1
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		  configure libtool stamp-h.in

configure : VERSION

all : .configure.patch

.configure.patch : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

## Makefile.am ends here
