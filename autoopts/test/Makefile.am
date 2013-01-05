##  -*- Mode: Makefile -*-
##
## Makefile.am
##
## Author:          Bruce Korb <bkorb@gnu.org>
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2013 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following md5sums:
##
##  43b91e8ca915626ed3818ffb1b71248b pkg/libopts/COPYING.gplv3
##  06a1a2e4760c90ea5e1dad8dfaac4d39 pkg/libopts/COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd

libsrc     = libopts-@AO_CURRENT@.@AO_REVISION@.@AO_AGE@.tar.gz
EXTRA_DIST = defs.in stdopts.def $(TESTS)
TESTS      = \
    alias.test   argument.test cfg-edit.test  cond.test      \
    config.test  doc.test      enums.test     equiv.test     \
    errors.test  getopt.test   handler.test   immediate.test \
    keyword.test library.test  main.test      nested.test    \
    nls.test     rc.test       shell.test     stdopts.test   \
    time.test    usage.test    vendor.test    vers.test

getopt.test : ../$(libsrc)
../$(libsrc):
	cd .. ; $(MAKE) $(libsrc)

testsubdir = ./testdir

TESTS_ENVIRONMENT = TERM='' top_builddir="$(top_builddir)"

distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

perm-stamp :
	@-cd $(srcdir) ; chmod +x *.test 2>/dev/null

verbose :
	rm -rf FAILURES testdir ; VERBOSE=true ; export VERBOSE ; \
	$(MAKE) check TESTS="$(TESTS)"

# Makefile.am ends here
