##  -*- Mode: Makefile -*-
##
## Makefile.am
##
## Time-stamp:      "2008-11-01 18:47:24 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2008/11/02 18:51:00 $
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is copyright (c) 1992-2007 by Bruce Korb - all rights reserved
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
##  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
##  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
#
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.17 2008/11/02 18:51:00 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs.in stdopts.def $(TESTS)
TESTS      = nested.test cfg-edit.test \
    argument.test cond.test    config.test enums.test   equiv.test \
    errors.test   getopt.test  guile.test  handler.test immediate.test \
    keyword.test  library.test main.test   nls.test     rc.test \
    shell.test    stdopts.test time.test   usage.test   vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = set -a ; \
	CFLAGS='$(DEFS) $(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' \
	testsubdir=$(testsubdir)
#`
distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

perm-stamp :
	@-cd $(srcdir) ; chmod +x *.test 2>/dev/null

verbose :
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS="$(TESTS)"

# Makefile.am ends here
