##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2010-07-08 21:42:17 bkorb"
## Author:	    Bruce Korb <bkorb@gnu.org>
##
## This file is part of AutoGen.
## AutoGen Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
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
## ---------------------------------------------------------------------
TARG = getdefs

bin_PROGRAMS    = getdefs
gdsrcs          = getdefs.h proto.h getdefs.c gdemit.c gdinit.c
getdefs_SOURCES = proto.h
BUILT_SOURCES   = gd.c
nodist_getdefs_SOURCES = $(BUILT_SOURCES)

SUBDIRS         = test
EXTRA_DIST      = opts.def $(gdsrcs)
RUNAG           = $(AGexe)
AGEXE_LIB       = $(RUNAG) -MF$@ -L$(top_srcdir)/autoopts
getdefs_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
DISTCLEANFILES  = $(DOCFILES) $(nodist_getdefs_SOURCES) *-stamp
INCLUDES        = @INCLIST@
AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns >/dev/null && pwd`:$$PATH ; \
  export top_builddir top_srcdir PATH

all : gen
gen : $(BUILT_SOURCES) $(DOCFILES)

gd.c : $(gdsrcs)
	( echo '#define DEFINE' ; \
	  for f in config.h opts.c $(gdsrcs) ; \
	  do echo "#include \"$$f\"" ; done ) > $@

$(getdefs_OBJECTS) opts.h opts.c : stamp-opts
stamp-opts : opts.def $(CLexe) $(AGexe)
	$(AG_ENV) ; $(AGEXE_LIB) $(srcdir)/opts.def

$(AGexe) :
	@cd ../agen5 ; $(MAKE) $(AGnam)

$(CLexe) :
	@cd ../columns ; $(MAKE) $(CLnam)

$(DOCFILES) : stamp-doc
stamp-doc : $(CLexe) $(TARG)$(EXEEXT) $(AGexe)
	$(AG_ENV) ; $(AGEXE_LIB) -b$(TARG) -Tagman1 $(srcdir)/opts.def ; \
	$(AGEXE_LIB) -b$(TARG) -Taginfo -DLEVEL=section $(srcdir)/opts.def

.NOTPARALLEL:

# Makefile.am ends here
