##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <bkorb@gnu.org>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 2000/09/11 01:03:44 $
##            by:    Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.17 2000/09/11 01:03:44 bkorb Exp $
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
	@chmod u+w ${top_srcdir}/doc/autogen.texi
	echo '@setfilename completely.bogus' > ${top_srcdir}/doc/autogen.texi
	@touch -t 200001010000 ${top_srcdir}/doc/autogen.texi

## Makefile.am ends here
