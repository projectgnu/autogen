##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-07-30 20:23:04 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.5 2002/07/31 03:25:48 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

AGEXE          = `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
DISTCLEANFILES = autogen.lsm
EXTRA_DIST     = lsm.tpl autogen.spec.in

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/lsm.tpl $(top_builddir)/agen5/opts.def

##  The following targets only work from CVS sources
##
gnudoc :
	pkgdir=`pwd` ; cd $(top_builddir)/doc ; $(SHELL) $${pkgdir}/mkgnudoc.sh

## Makefile.am ends here
