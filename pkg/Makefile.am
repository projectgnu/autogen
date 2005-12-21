##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-12-15 15:17:51 bkorb"
## Copyright (c):   2003-2005 by Bruce Korb
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.7 2005/12/21 22:13:35 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

DISTCLEANFILES = autogen.lsm
PKG_FILES      = mkpkg.linux mkpkg.sh mkpkg.sun
LIBOPTS_FILES  = libopts/COPYING.mbsd libopts/README libopts/mklibsrc.sh \
		 libopts/libopts-add.m4
TPL_FILES      = lsm.tpl spec.tpl gnudir.tpl gnudoc.tpl
EXTRA_DIST     = $(TPL_FILES) $(LIBOPTS_FILES) $(PKG_FILES) mkgnudoc.sh
DOCENV         = \
	MAKE="$(MAKE)" \
	PACKAGE="$(PACKAGE)" \
	PACKAGE_BUGREPORT="$(PACKAGE_BUGREPORT)" \
	PACKAGE_NAME="$(PACKAGE_NAME)" \
	PACKAGE_STRING="$(PACKAGE_STRING)" \
	PACKAGE_TARNAME="$(PACKAGE_TARNAME)" \
	PACKAGE_VERSION="$(PACKAGE_VERSION)"

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -T $(srcdir)/lsm.tpl $(top_srcdir)/agen5/opts.def

autogen.spec : $(top_srcdir)/VERSION spec.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -T $(srcdir)/spec.tpl $(top_srcdir)/agen5/opts.def

package :
	top_builddir=$(top_builddir) \
	top_srcdir=$(top_srcdir) \
	srcdir=$(srcdir) \
	AGexe=$(AGexe) \
	CLexe=$(CLexe) \
	prefix=$(prefix) \
	DESTDIR=`cd $(top_builddir) > /dev/null ; pwd`/stage-pkg \
	$(DOCENV) $(SHELL) $(srcdir)/mkpkg.sh

pkg     : package

##  The following targets only work from CVS sources
##
gnudoc  :
	pkgsrcdir=`cd $(srcdir) ; pwd` ; \
	cd $(top_builddir)/doc ; \
	$(DOCENV) $(SHELL) $${pkgsrcdir}/mkgnudoc.sh \
		The Automated Program Generator

## Makefile.am ends here
