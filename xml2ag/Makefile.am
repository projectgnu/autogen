##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 2002-2003 Bruce Korb
##
## Time-stamp:      "2003-04-21 18:03:22 bkorb"
## Author:	        Bruce Korb <bkorb@gnu.org>
## Created:	        Jun 10 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.11 2003/04/22 01:40:20 bkorb Exp $
## ---------------------------------------------------------------------
TARG = xml2ag

bin_PROGRAMS    = xml2ag
csrcs           = xmlopts.h xml2ag.c xmlopts.c fork.c
nodist_xml2ag_SOURCES = x.c

## SUBDIRS      = test
EXTRA_DIST      = xmlopts.def fork.tpl $(csrcs)
AGEXE           = $(top_builddir)/agen5/@AGEXE@
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la $(LIBXML2_LIBS)
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_xml2ag_SOURCES) *-stamp
INCLUDES        = @INCLIST@ $(LIBXML2_CFLAGS)

all : gen
gen : $(nodist_xml2ag_SOURCES) $(DOCFILES)

x.c : $(csrcs)
	for f in $(csrcs) ; do echo "#include \"$$f\"" ; done > $@

xmlopts.c $(xml2ag_OBJECTS) : xmlopts.h

xmlopts.h : xmlopts.def
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) $(srcdir)/xmlopts.def

fork.c : fork.tpl xmlopts.def
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -Tfork.tpl $(srcdir)/xmlopts.def

$(DOCFILES) : doc-stamp

doc-stamp : xmlopts.def $(TARG)$(EXEEXT)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	$(AGEXE) -Tagman1 -b$(TARG) $(srcdir)/xmlopts.def ; \
	$(AGEXE) -Taginfo -b$(TARG) -DLEVEL=section $(srcdir)/xmlopts.def
	@touch $@

# Makefile.am ends here
