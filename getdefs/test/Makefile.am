##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <autogen@linuxbox.com>
## Maintainer:      Bruce Korb <autogen@linuxbox.com>
## Created:         Fri July 18
## Last Modified:   Thu Jul 29 13:42:18 1999				      
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.1 1999/10/31 16:22:15 bruce Exp $
## ---------------------------------------------------------------------

TESTS      = subblock.test
EXTRA_DIST = defs stress.def bootstrap.dir $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

TESTS_ENVIRONMENT = GETDEFS=$(top_builddir)/getdefs/getdefs \
	testsubdir=$(testsubdir) srcdir=$(srcdir) \
	top_srcdir=$(top_srcdir)

testsubdir = ./testdir

distclean-local:
	-rm -rf $(testsubdir) FAILURES

# Makefile.am ends here
