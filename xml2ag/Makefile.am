##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2012-03-31 13:16:15 bkorb"
## Last Modified:   Thu Jul 29 13:42:18 1999
## Author:          Bruce Korb <bkorb@gnu.org>
##
## This file is part of AutoGen.
##  AutoGen Copyright (c) 1992-2012 by Bruce Korb - all rights reserved
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
TARG = xml2ag

bin_PROGRAMS    = xml2ag
csrcs           = xml2ag.c
gsrcs           = xmlopts.c fork.c
BUILT_SOURCES   = x.c
nodist_xml2ag_SOURCES = $(BUILT_SOURCES)
SUBDIRS         = test
EXTRA_DIST      = xmlopts.def fork.tpl fork.c xml2ag.c
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la $(LIBXML2_LIBS)
man_MANS        = $(TARG).1
DOCFILES        = invoke-$(TARG).texi invoke-$(TARG).menu $(TARG).1
DISTCLEANFILES  = $(DOCFILES) $(nodist_xml2ag_SOURCES) stamp-*
INCLUDES        = @INCLIST@ $(LIBXML2_CFLAGS)
AGexe           = $(top_builddir)/agen5/autogen
CLexe           = $(top_builddir)/columns/columns
BOOTENV     = top_srcdir="$(top_srcdir)" top_builddir="$(top_builddir)" \
	PATH=`cd ../columns;pwd`:"$$PATH" CLexe="$(CLexe)"

RUNAG       = $(BOOTENV) $(AGexe) $(AGDEPS) \
	-L$(top_srcdir)/autoopts/tpl -L$(top_builddir)/autoopts/tpl \
	--definition=$(srcdir)/xmlopts.def

if AMDEP
CLEAN_RULES = \
	clean-stamp-opts clean-stamp-texi clean-stamp-man clean-stamp-fork
clean-local : $(CLEAN_RULES)

@am__include@ @am__quote@./$(DEPDIR)/stamp-opts.d@am__quote@
@am__include@ @am__quote@./$(DEPDIR)/stamp-texi.d@am__quote@
@am__include@ @am__quote@./$(DEPDIR)/stamp-man.d@am__quote@
@am__include@ @am__quote@./$(DEPDIR)/stamp-fork.d@am__quote@
AGDEPS      = -MF$(DEPDIR)/$@.d -MT$@ -MP
else
AGDEPS      =
endif

all 	    : gen
gen 	    : $(gsrcs) $(DOCFILES)

$(getdefs_OBJECTS) xmlopts.c xmlopts.h : stamp-opts
stamp-opts  : xmlopts.def $(CLexe)
	$(RUNAG)

fork.c      : stamp-fork
stamp-fork  : fork.tpl xmlopts.def $(CLexe)
	$(RUNAG) -L$(srcdir) -Tfork.tpl

invoke-$(TARG).texi invoke-$(TARG).menu : stamp-texi
stamp-texi  : xmlopts.def $(TARG)$(EXEEXT) ../columns/columns$(EXEEXT)
	$(RUNAG) -Tagtexi-cmd -DLEVEL=section

$(TARG).1   : stamp-man
stamp-man   : xmlopts.def $(TARG)$(EXEEXT) ../columns/columns$(EXEEXT)
	$(RUNAG) -Tagman-cmd

$(CLexe)    :
	cd ../columns ; $(MAKE) $(CLnam)

x.c 	    : $(gsrcs) $(csrcs)
	exec > $@ ; \
	echo '#undef   PKGDATADIR' ; \
	echo '#define  PKGDATADIR "$(pkgdatadir)"' ; \
	echo ; echo '#define  DEFINING 1' ; \
	echo '#include "autoopts/project.h"' ; \
	for f in $(gsrcs) $(csrcs) ; \
	do echo "#include \"$$f\"" ; done

.NOTPARALLEL:

# Makefile.am ends here
