##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:  ""
## Author:	    Bruce Korb <bkorb@gnu.org>
## Created:	    Jun 10 2002
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.4 2002/06/11 01:27:19 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = xml2ag

xml2ag_SOURCES  = xml2ag.c
nodist_xml2ag_SOURCES = xmlopts.c xmlopts.h fork.c

BUILT_SOURCES   = xmlopts.c xmlopts.h fork.c
EXTRA_DIST      = xmlopts.def fork.tpl
AGEXE           = $(top_builddir)/agen5/@AGEXE@ -L$(top_srcdir)/agen5
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la
man_MANS        = xml2ag.1
GENFILES        = xml2ag.texi xml2ag.menu xml2ag.1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(GENFILES) $(nodist_xml2ag_SOURCES)

xmlopts.c $(OBJ) : xmlopts.h

xmlopts.h : xmlopts.def
	top_builddir=$(top_builddir) COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -L$(top_srcdir)/autoopts xmlopts.def

fork.c : fork.tpl xmlopts.def
	top_builddir=$(top_builddir) COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -T fork.tpl xmlopts.def

$(GENFILES) : gen-stamp

gen-stamp : xmlopts.def
	top_builddir=$(top_builddir) \
	$(AGEXE) -L$(top_srcdir)/autoopts -Taginfo -bxml2ag -DLEVEL=section \
		$(top_srcdir)/xml2ag/xmlopts.def
	$(AGEXE) -L$(top_srcdir)/autoopts -T agman1 -b xml2ag $(srcdir)/xmlopts.def
	@touch $@

clean : xml2ag-clean
distclean : xml2ag-clean

xml2ag-clean :
	rm -f *-stamp
