##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   $Date: 2002/05/30 01:31:08 $
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.4 2002/05/30 01:31:08 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
testsubdir = ./testdir
CPPFLAGS   = @CPPFLAGS@
TESTS      = argument.test cond.test equiv.test errors.test guile.test \
	handler.test keyword.test rc.test shell.test stdopts.test vers.test

MAINTAINERCLEANFILES	= Makefile.in

TESTS_ENVIRONMENT = \
	CC="$(CC)" \
	CFLAGS='@CFLAGS@' \
	PATH="`cd $(top_builddir)/columns ; pwd`:$$PATH" \
	AGEN="`cd $(top_builddir)/agen5 ; pwd`/@AGEXE@" \
	COLUMNS_EXE="`cd $(top_builddir)/columns ; pwd`/@COLEXE@" \
	LIB="@LDFLAGS@ `cd $(top_builddir)/autoopts/.libs/; \
	echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a ` $(LIBS)" \
	INC="@INCLIST@ $(CPPFLAGS)" \
	testsubdir=$(testsubdir) \
	srcdir=`pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
