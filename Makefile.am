##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <bkorb@gnu.org>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 2000/09/11 01:43:45 $
##            by:    Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.18 2000/09/11 01:43:45 bkorb Exp $
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

##  The following is a "nice try", but no cigar.  The make rules are *really*
##  insistent that they get a valid .texi file distributed.  Grr. 
## 
## dist-hook:
## 	@chmod u+w ${top_srcdir}/doc/autogen.texi
## 	echo '@setfilename completely.bogus' > ${top_srcdir}/doc/autogen.texi
## 	@touch -t 200001010000 ${top_srcdir}/doc/autogen.texi

## Makefile.am ends here
