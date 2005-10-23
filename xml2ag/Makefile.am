##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 2002-2005 Bruce Korb
##
## Time-stamp:      "2005-10-22 19:16:41 bkorb"
## Last Modified:   Thu Jul 29 13:42:18 1999				      
## Author:          Bruce Korb <bkorb@gnu.org>
## Created:         Jun 10 2002
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.7 2005/10/23 02:40:03 bkorb Exp $
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
MAINTAINERCLEANFILES = Makefile.in
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

# Makefile.am ends here
