##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 1992-2002 Bruce Korb
##
## Time-stamp:      "2002-07-08 18:42:28 bkorb"
## Author:	        Bruce Korb <bkorb@gnu.org>
## Created:	        Jun 10 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.7 2002/07/09 02:32:25 bkorb Exp $
## ---------------------------------------------------------------------
TARG = xml2ag

bin_PROGRAMS    = xml2ag
nodist_xml2ag_SOURCES = xmlopts.c xmlopts.h fork.c
xml2ag_SOURCES  = xml2ag.c

## SUBDIRS      = test
EXTRA_DIST      = xmlopts.def fork.tpl
AGEXE           = $(top_builddir)/agen5/@AGEXE@ -L$(top_srcdir)/agen5
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la @XML2AG_LDFLAGS@
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_xml2ag_SOURCES) *-stamp
INCLUDES        = @INCLIST@ @XML2AG_CPPFLAGS@

all : gen
gen : $(nodist_xml2ag_SOURCES) $(DOCFILES)

xmlopts.c $(xml2ag_OBJECTS) : xmlopts.h

xmlopts.h : xmlopts.def
	top_builddir=$(top_builddir) COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -L$(top_srcdir)/autoopts $(srcdir)/xmlopts.def

fork.c : fork.tpl xmlopts.def
	top_builddir=$(top_builddir) COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -T fork.tpl xmlopts.def

$(DOCFILES) : doc-stamp

doc-stamp : xmlopts.def $(TARG)$(EXEEXT)
	top_builddir=$(top_builddir) \
	$(AGEXE) -L$(top_srcdir)/autoopts -Taginfo -b$(TARG) -DLEVEL=section \
		$(top_srcdir)/$(TARG)/xmlopts.def
	$(AGEXE) -L$(top_srcdir)/autoopts -T agman1 -b$(TARG) $(srcdir)/xmlopts.def
	@touch $@

# Makefile.am ends here
