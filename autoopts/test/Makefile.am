##  -*- Mode: Makefile -*- 
##
## Makefile.am
##
## Time-stamp:      "2004-10-27 19:57:19 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2005/01/01 00:21:00 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.1 2005/01/01 00:21:00 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
MAINTAINERCLEANFILES	= Makefile.in
TESTS      = \
    argument.test cond.test    enums.test   equiv.test     errors.test  \
    getopt.test   guile.test   handler.test immediate.test keyword.test \
    main.test     nls.test     rc.test      shell.test     stdopts.test \
    usage.test    vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = AGexe=$(AGexe) CLexe=$(CLexe) GDexe=$(GDexe) \
	CC="$(CC)" CFLAGS='$(CFLAGS)' \
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

# Makefile.am ends here
