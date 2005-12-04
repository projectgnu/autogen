##  -*- Mode: Makefile -*- 
##
## Makefile.am
##
## Time-stamp:      "2005-12-03 15:03:39 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2005/12/04 00:57:31 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.8 2005/12/04 00:57:31 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
MAINTAINERCLEANFILES	= Makefile.in
TESTS      = nested.test \
    argument.test cond.test   config.test enums.test   equiv.test \
    errors.test   getopt.test guile.test  handler.test immediate.test \
    keyword.test  main.test   nls.test    rc.test      shell.test \
    stdopts.test  usage.test  vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = set -a ; AGexe=$(AGexe) CLexe=$(CLexe) GDexe=$(GDexe) \
	CC="$(CC)" CFLAGS='$(DEFS) $(CFLAGS)' LIBGUILE_PATH=$(LIBGUILE_PATH) \
	LD_LIBRARY_PATH=$(LIBGUILE_PATH):$${LD_LIBRARY_PATH-../.libs} \
	LD_RUN_PATH=$(LIBGUILE_PATH):$${LD_RUN_PATH-../.libs} \
	PATH="`cd $(top_builddir)/columns > /dev/null ; pwd`:$$PATH" \
	PATH="$(LIBGUILE_PATH)/../bin:$$PATH" \
	LIB="@LDFLAGS@ `cd $(top_builddir)/autoopts/.libs/ > /dev/null ; \
	echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a ` $(LIBS)" \
	INC="-I`cd $(top_builddir)/autoopts > /dev/null ; pwd` \
	-I`cd $(top_builddir) > /dev/null ; pwd` \
	-I`cd $(top_srcdir) > /dev/null ; pwd` \
	    $(INCLIST) $(CPPFLAGS) $(LIBGUILE_CFLAGS) $(LIBGUILE_LIBS)" \
	testsubdir=$(testsubdir) \
	srcdir=`cd $(srcdir) > /dev/null ; pwd` \
	top_srcdir=`\cd $(top_srcdir) > /dev/null ; pwd` \
	top_builddir=`\cd $(top_builddir) > /dev/null ; pwd`
#`
distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

verbose :
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS=$(TESTS)

# Makefile.am ends here
