##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2002-10-12 14:29:43 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Sun Jul 28 20:37 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.7 2002/10/12 21:31:58 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

AGEXE          = `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
DISTCLEANFILES = autogen.lsm
EXTRA_DIST     = lsm.tpl autogen.spec.in libopts/*

all : $(DISTCLEANFILES)

autogen.lsm : $(top_srcdir)/VERSION lsm.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/lsm.tpl $(top_builddir)/agen5/opts.def

##  The following targets only work from CVS sources
##
gnudoc :
	pkgsrcdir=`cd $(srcdir) ; pwd` ; export pkgsrcdir ; \
	cd $(top_builddir)/doc ; \
	$(SHELL) $${pkgsrcdir}/mkgnudoc.sh

## Makefile.am ends here
