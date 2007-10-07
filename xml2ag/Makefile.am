##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 2002-2007 Bruce Korb
##
## Time-stamp:      "2007-07-04 11:43:00 bkorb"
## Last Modified:   Thu Jul 29 13:42:18 1999
## Author:          Bruce Korb <bkorb@gnu.org>
##              by: bkorb
##
## This file is part of AutoGen.
##  AutoGen copyright (c) 1992-2007 Bruce Korb - all rights reserved
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
## $Id: Makefile.am,v 4.12 2007/10/07 16:54:54 bkorb Exp $
## ---------------------------------------------------------------------
TARG = xml2ag

bin_PROGRAMS    = xml2ag
csrcs           = xmlopts.h xml2ag.c xmlopts.c fork.c
nodist_xml2ag_SOURCES = x.c
BUILT_SOURCES   = x.c xmlopts.c xmlopts.h
SUBDIRS         = test
EXTRA_DIST      = xmlopts.def fork.tpl $(csrcs)
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la $(LIBXML2_LIBS)
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
DISTCLEANFILES  = $(DOCFILES) $(nodist_xml2ag_SOURCES) *-stamp
INCLUDES        = @INCLIST@ $(LIBXML2_CFLAGS)
AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns;pwd`:$$PATH ; \
  export top_builddir top_srcdir PATH

run_AG = $(AGexe) -L$(top_srcdir)/autoopts --definition=$(srcdir)/xmlopts.def

all : gen
gen : $(nodist_xml2ag_SOURCES) $(DOCFILES)

x.c : $(csrcs)
	for f in config.h $(csrcs) ; do echo "#include \"$$f\"" ; done > $@

xmlopts.c $(xml2ag_OBJECTS) : xmlopts.h

xmlopts.h : xmlopts.def $(CLexe)
	$(AG_ENV) ; $(run_AG)

fork.c : fork.tpl xmlopts.def $(CLexe)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	CLexe=$(CLexe) \
	$(run_AG) -L$(srcdir) -Tfork.tpl

$(DOCFILES) : doc-stamp
$(CLexe) :
	cd ../columns ; $(MAKE) $(CLnam)

doc-stamp : xmlopts.def $(TARG)$(EXEEXT) ../columns/columns$(EXEEXT)
	$(AG_ENV) ; \
	$(run_AG) -Tagman1 -b$(TARG) ; \
	$(run_AG) -Taginfo -b$(TARG) -DLEVEL=section
	@touch $@

.NOTPARALLEL:

# Makefile.am ends here
