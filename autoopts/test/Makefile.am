##  -*- Mode: Makefile -*- 
##
## Makefile.am
##
## Time-stamp:      "2003-03-12 19:40:58 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2003/03/13 04:08:04 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.9 2003/03/13 04:08:04 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
testsubdir = ./testdir
TESTS      = argument.test cond.test equiv.test errors.test guile.test \
	handler.test keyword.test rc.test shell.test stdopts.test usage.test \
    vers.test

MAINTAINERCLEANFILES	= Makefile.in

TESTS_ENVIRONMENT = \
	CC="$(CC)" CFLAGS='$(CFLAGS)' \
	LD_LIBRARY_PATH=$(LIBGUILE_PATH):$${LD_LIBRARY_PATH-..} \
	PATH="`cd $(top_builddir)/columns > /dev/null ; pwd`:$$PATH" \
	AGEN="`cd $(top_builddir)/agen5   > /dev/null ; pwd`/$(AGEXE)" \
	COLUMNS_EXE="`cd $(top_builddir)/columns > /dev/null ; pwd`/$(COLEXE)" \
	LIB="@LDFLAGS@ `cd $(top_builddir)/autoopts/.libs/; \
	echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a ` $(LIBS)" \
	INC="$(INCLIST) $(CPPFLAGS) $(LIBGUILE_CFLAGS) $(LIBGUILE_LIBS)" \
	testsubdir=$(testsubdir) \
	srcdir=`pwd` \
	top_srcdir=`cd $(top_srcdir) > /dev/null ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
