##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 1998/06/01 18:50:45 $
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.3 1998/06/01 18:50:45 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

SUBDIRS = compat config autoopts src tests doc

EXTRA_DIST	= NOTES VERSION TODO bootstrap configure release
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config.h.in \
		 configure libtool stamp-h.in

VERSION: $(top_srcdir)/config/VERSION.tpl $(top_srcdir)/src/opts.def
	if [ -x $(top_builddir)/src/autogen ] ; then \
	$(top_builddir)/src/autogen -L $(top_srcdir)/config -T VERSION.tpl \
		$(top_srcdir)/src/opts.def ; \
	else touch $@ ; fi

## Makefile.am ends here
