##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 2002-2004 Bruce Korb
##
## Time-stamp:      "2004-05-29 12:45:29 bkorb"
## Copyright (c):   2003-2004 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Created:         Jun 10 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.17 2004/07/02 18:30:56 bkorb Exp $
## ---------------------------------------------------------------------
TARG = xml2ag

bin_PROGRAMS    = xml2ag
csrcs           = xmlopts.h xml2ag.c xmlopts.c fork.c
nodist_xml2ag_SOURCES = x.c
BUILT_SOURCES   = x.c xmlopts.c xmlopts.h
SUBDIRS         = test
EXTRA_DIST      = xmlopts.def fork.tpl $(csrcs)
AGEXE           = $(top_builddir)/agen5/@AGEXE@
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la $(LIBXML2_LIBS)
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_xml2ag_SOURCES) *-stamp
INCLUDES        = @INCLIST@ $(LIBXML2_CFLAGS)
AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns;pwd`:$$PATH ; \
  export top_builddir top_srcdir PATH
COL_EXE         = `cd ../columns;pwd`/columns

all : gen
gen : $(nodist_xml2ag_SOURCES) $(DOCFILES)

x.c : $(csrcs)
	for f in $(csrcs) ; do echo "#include \"$$f\"" ; done > $@

xmlopts.c $(xml2ag_OBJECTS) : xmlopts.h

xmlopts.h : xmlopts.def ../columns/columns$(EXEEXT)
	$(AG_ENV) ; $(AGEXE) $(srcdir)/xmlopts.def

fork.c : fork.tpl xmlopts.def ../columns/columns$(EXEEXT)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	COLUMNS_EXE=$(COL_EXE) \
	$(AGEXE) -Tfork.tpl $(srcdir)/xmlopts.def

$(DOCFILES) : doc-stamp
../columns/columns$(EXEEXT) :
	cd ../columns ; $(MAKE) columns$(EXEEXT)

doc-stamp : xmlopts.def $(TARG)$(EXEEXT) ../columns/columns$(EXEEXT)
	$(AG_ENV) ; $(AGEXE) -Tagman1 -b$(TARG) $(srcdir)/xmlopts.def ; \
	$(AGEXE) -Taginfo -b$(TARG) -DLEVEL=section $(srcdir)/xmlopts.def
	@touch $@

# Makefile.am ends here
