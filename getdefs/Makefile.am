##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 1992-2003 Bruce Korb
##
## Time-stamp:      "2003-12-20 10:20:10 bkorb"
## Author:	        Bruce Korb <bkorb@gnu.org>
## Created:	        Mon Jun 30, 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.12 2003/12/27 15:06:40 bkorb Exp $
## ---------------------------------------------------------------------
TARG = getdefs

bin_PROGRAMS    = getdefs
gdsrcs          = getdefs.h getdefs.c gdemit.c gdinit.c opts.c
getdefs_SOURCES = proto.h
nodist_getdefs_SOURCES = gd.c opts.h

SUBDIRS         = test
BUILT_SOURCES   = opts.c opts.h
EXTRA_DIST      = opts.def $(gdsrcs)
AGEXE           = $(top_builddir)/agen5/@AGEXE@
AGEXE_LIB       = $(AGEXE) -L$(top_srcdir)/autoopts
getdefs_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_getdefs_SOURCES) *-stamp
INCLUDES        = @INCLIST@
COLUMNS_EXE     = $(top_builddir)/columns/columns

all : gen
gen : $(BUILT_SOURCES) $(DOCFILES)

opts.c $(getdefs_OBJECTS) : opts.h
gd.c : $(gdsrcs)
	( echo '#define DEFINE' ; \
	  for f in $(gdsrcs) ; do echo "#include \"$$f\"" ; done ) > $@

opts.h : opts.def $(COLUMNS_EXE) $(AGEXE)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	COLUMNS_EXE=$(COLUMNS_EXE) ; \
	export top_builddir top_srcdir COLUMNS_EXE ; \
	$(AGEXE_LIB) $(srcdir)/opts.def

$(DOCFILES) : doc-stamp

$(AGEXE) :
	@cd ../agen5 ; $(MAKE) autogen

$(COLUMNS_EXE) :
	@cd ../columns ; $(MAKE) columns

doc-stamp : opts.def $(TARG)$(EXEEXT) $(AGEXE)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) ; \
	export top_builddir top_srcdir ; \
	$(AGEXE_LIB) -Tagman1 -b$(TARG) $(srcdir)/opts.def ; \
	$(AGEXE_LIB) -Taginfo -b$(TARG) -DLEVEL=section $(srcdir)/opts.def
	@touch $@

# Makefile.am ends here
