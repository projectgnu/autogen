##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
## Copyright (c):   2003 by Bruce Korb
## Time-stamp:      "2003-02-19 20:27:07 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.8 2003/02/28 21:41:35 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

AGEXE          = `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
DISTCLEANFILES = autogen.lsm
PKG_FILES      = mkpkg.linux mkpkg.sh mkpkg.sun
LIBOPTS_FILES  = libopts/COPYING.mbsd libopts/README libopts/mklibsrc.sh
EXTRA_DIST     = lsm.tpl autogen.spec.in $(LIBOPTS_FILES) $(PKG_FILES)

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/lsm.tpl $(top_builddir)/agen5/opts.def

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
