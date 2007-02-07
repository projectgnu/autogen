##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2007-02-04 11:07:12 bkorb"
## Copyright (c):   2003-2007 by Bruce Korb
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.11 2007/02/07 01:57:59 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

DISTCLEANFILES = autogen.lsm
DEBIAN_FILES   = mkpkg.debian debian
PKG_FILES      = mkpkg.linux mkpkg.sh mkpkg.sun pkg-env.in $(DEBIAN_FILES)
LIBOPTS_FILES  = libopts/COPYING.mbsd libopts/README libopts/mklibsrc.sh \
		 libopts/libopts-add.m4
TPL_FILES      = lsm.tpl spec.tpl gnudir.tpl gnudoc.tpl
EXTRA_DIST     = $(TPL_FILES) $(LIBOPTS_FILES) $(PKG_FILES) mkgnudoc.sh
DOCENV         = MAKE="$(MAKE)"

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -L$(top_srcdir)/autoopts -T $(srcdir)/lsm.tpl \
	    $(top_srcdir)/agen5/opts.def

autogen.spec : $(top_srcdir)/VERSION spec.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGexe) -T $(srcdir)/spec.tpl $(top_srcdir)/agen5/opts.def

package :
	pkgtype=$(pkgtype) \
	DESTDIR=`cd $(top_builddir) > /dev/null ; pwd`/stage-pkg \
	$(DOCENV) $(SHELL) $(srcdir)/mkpkg.sh

pkg     : package

##  The following targets only work from CVS sources
##
gnudoc  :
	pkgsrcdir=`cd $(srcdir) ; pwd` ; \
	cd $(top_builddir)/doc ; \
	$(DOCENV) $(SHELL) $${pkgsrcdir}/mkgnudoc.sh \
		"$(PACKAGE)" "$(PACKAGE_VERSION)" \
		The Automated Program Generator

.NOTPARALLEL:

## Makefile.am ends here
