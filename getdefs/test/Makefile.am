##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Time-stamp:      "2009-07-22 18:51:43 bkorb"
##            by: bkorb
## This file is part of AutoGen.
## AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.6 2009/07/23 02:05:55 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = cfg.test index.test option.test subblock.test
EXTRA_DIST = defs $(TESTS)
testsubdir = ./testdir

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) \
	testsubdir=$(testsubdir) srcdir=$(srcdir) \
	GDexe=$(GDexe) AGexe=$(AGexe) CLexe=$(CLexe)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test

# Makefile.am ends here
