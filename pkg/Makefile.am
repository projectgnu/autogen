##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-02-01 13:22:53 bkorb"
## Copyright (c):   2003-2004 by Bruce Korb
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.11 2004/10/02 02:20:41 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

AGEXE          = `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
DISTCLEANFILES = autogen.lsm
PKG_FILES      = mkpkg.linux mkpkg.sh mkpkg.sun
LIBOPTS_FILES  = libopts/COPYING.mbsd libopts/README libopts/mklibsrc.sh
TPL_FILES      = lsm.tpl spec.tpl gnudir.tpl gnudoc.tpl
EXTRA_DIST     = $(TPL_FILES) $(LIBOPTS_FILES) $(PKG_FILES) mkgnudoc.sh

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/lsm.tpl $(top_builddir)/agen5/opts.def

autogen.spec : $(top_srcdir)/VERSION spec.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/spec.tpl $(top_builddir)/agen5/opts.def

package :
	top_builddir=$(top_builddir) \
	top_srcdir=$(top_srcdir) \
	srcdir=$(srcdir) \
	AGEXE=$(AGEXE) \
	prefix=$(prefix) \
	MAKE="$(MAKE)" \
	DESTDIR=`cd $(top_builddir) > /dev/null ; pwd`/=stage \
	$(SHELL) mkpkg.sh

##  The following targets only work from CVS sources
##
gnudoc :
	pkgsrcdir=`cd $(srcdir) ; pwd` ; export pkgsrcdir ; \
	cd $(top_builddir)/doc ; \
	$(SHELL) $${pkgsrcdir}/mkgnudoc.sh

## Makefile.am ends here
