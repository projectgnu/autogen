##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Copyright (c):   2003-2004 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   Thu Jul 29 13:42:18 1999				      
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.3 2004/10/30 20:43:57 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = convert.test
EXTRA_DIST = $(TESTS)

MAINTAINERCLEANFILES = Makefile.in

testsubdir = ./testdir

TESTS_ENVIRONMENT = \
	X2Aexe=`\cd $(top_builddir)/xml2ag ; pwd`/xml2ag$(EXEEXT) \
	testsubdir=$(testsubdir) \
	srcdir=$(srcdir) top_srcdir=$(top_srcdir)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test

# Makefile.am ends here

 XML2AG=