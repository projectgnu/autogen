##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    $Date: 1998/05/26 18:09:24 $
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.2 1998/05/26 18:09:24 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@
VERSION = @VERSION@

SUBDIRS = compat config autoopts src tests doc

EXTRA_DIST = VERSION TODO bootstrap

VERSION: $(top_srcdir)/config/VERSION.tpl $(top_srcdir)/src/opts.def
	if [ -x $(top_builddir)/src/autogen ] ; then \
	$(top_builddir)/src/autogen -L $(top_srcdir)/config -T VERSION.tpl \
		$(top_srcdir)/src/opts.def ; \
	else touch $@ ; fi

## Makefile.am ends here
