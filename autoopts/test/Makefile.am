##  -*- Mode: Makefile -*- 
##
## Makefile.am
##
## Time-stamp:      "2005-06-26 08:13:00 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2005/06/26 16:24:45 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.5 2005/06/26 16:24:45 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
MAINTAINERCLEANFILES	= Makefile.in
TESTS      = nested.test \
    argument.test cond.test   config.test enums.test   equiv.test \
    errors.test   getopt.test guile.test  handler.test immediate.test \
    keyword.test  main.test   nls.test    rc.test      shell.test \
    stdopts.test  usage.test  vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = AGexe=$(AGexe) CLexe=$(CLexe) GDexe=$(GDexe) \
	CC="$(CC)" CFLAGS='$(DEFS) $(CFLAGS)' \
	LD_LIBRARY_PATH=$(LIBGUILE_PATH):$${LD_LIBRARY_PATH-..} \
	PATH="`cd $(top_builddir)/columns > /dev/null ; pwd`:$$PATH" \
	LIB="@LDFLAGS@ `cd $(top_builddir)/autoopts/.libs/ > /dev/null ; \
	echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a ` $(LIBS)" \
	INC="-I`cd $(top_builddir)/autoopts > /dev/null ; pwd` \
	-I`cd $(top_builddir) > /dev/null ; pwd` \
	    $(INCLIST) $(CPPFLAGS) $(LIBGUILE_CFLAGS) $(LIBGUILE_LIBS)" \
	testsubdir=$(testsubdir) \
	srcdir=`pwd` \
	top_srcdir=`\cd $(top_srcdir) > /dev/null ; pwd` \
	top_builddir=`\cd $(top_builddir) > /dev/null ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

tests-current :
	if test -f .sdir ; then for f in $(TESTS) ; do \
	cp `cat .sdir`/$$f $$f ; done ; fi

verbose : tests-current
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS=$(TESTS)

# Makefile.am ends here
