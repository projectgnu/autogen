##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-10-27 19:36:44 bkorb"
## Copyright (c):   2003-2004 by Bruce Korb
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.1 2005/01/01 00:20:58 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

DISTCLEANFILES = autogen.lsm
PKG_FILES      = mkpkg.linux mkpkg.sh mkpkg.sun
LIBOPTS_FILES  = libopts/COPYING.mbsd libopts/README libopts/mklibsrc.sh
TPL_FILES      = lsm.tpl spec.tpl gnudir.tpl gnudoc.tpl
EXTRA_DIST     = $(TPL_FILES) $(LIBOPTS_FILES) $(PKG_FILES) mkgnudoc.sh

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -T $(srcdir)/lsm.tpl $(top_builddir)/agen5/opts.def

autogen.spec : $(top_srcdir)/VERSION spec.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -T $(srcdir)/spec.tpl $(top_builddir)/agen5/opts.def

package :
	top_builddir=$(top_builddir) \
	top_srcdir=$(top_srcdir) \
	srcdir=$(srcdir) \
	AGexe=$(AGexe) \
	CLexe=$(CLexe) \
	prefix=$(prefix) \
	MAKE="$(MAKE)" \
	DESTDIR=`cd $(top_builddir) > /dev/null ; pwd`/=stage \
	$(SHELL) mkpkg.sh

pkg     : package

##  The following targets only work from CVS sources
##
gnudoc :
	pkgsrcdir=`cd $(srcdir) ; pwd` ; export pkgsrcdir ; \
	cd $(top_builddir)/doc ; \
	$(SHELL) $${pkgsrcdir}/mkgnudoc.sh

## Makefile.am ends here
