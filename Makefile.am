##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <bkorb@gnu.org>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 2000/09/11 00:26:21 $
##            by:    Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.16 2000/09/11 00:26:21 bkorb Exp $
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

dist-hook:
	echo '@setfilename completely.bogus' > ${top_srcdir}/doc/autogen.texi
	@touch -t 200001010000 ${top_srcdir}/doc/autogen.texi

## Makefile.am ends here
