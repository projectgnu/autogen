##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Maintainer:	     Bruce Korb <korbb@datadesign.com>
## Created:	     Mon Jun 30 15:35:12 1997				
## Last Modified:    1998/04/09 at 13:18:30
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## @(#)Makefile.am	2.3
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

updates:
	@if [ ! -f autogen-@TM_VERSION@.tar.gz ] ; \
	then echo autogen-@TM_VERSION@.tar.gz does not exist ; \
	else \
		find * -name SCCS -prune -o -type f -newer \
			autogen-@TM_VERSION@.tar.gz -print ; \
	fi | while read f;do d=`dirname $$f` ; g=`basename $$f` ; \
	if [ -f $$d/SCCS/s.$$g ] ; then echo updated $$f ; fi ; done

## Makefile.am ends here
