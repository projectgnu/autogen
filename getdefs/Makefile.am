##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 1992-2004 Bruce Korb
##
## Time-stamp:      "2004-10-13 21:03:35 bkorb"
## Copyright (c):   1997-2004 by Bruce Korb
## Author:	        Bruce Korb <bkorb@gnu.org>
## Created:	        Mon Jun 30, 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.17 2004/10/15 01:48:34 bkorb Exp $
## ---------------------------------------------------------------------
TARG = getdefs

bin_PROGRAMS    = getdefs
gdsrcs          = getdefs.h getdefs.c gdemit.c gdinit.c opts.c
getdefs_SOURCES = proto.h
nodist_getdefs_SOURCES = gd.c opts.h

SUBDIRS         = test
BUILT_SOURCES   = opts.c opts.h gd.c
EXTRA_DIST      = opts.def $(gdsrcs)
AGEXE           = $(top_builddir)/agen5/@AGEXE@
AGEXE_LIB       = $(AGEXE) -L$(top_srcdir)/autoopts
getdefs_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_getdefs_SOURCES) *-stamp
INCLUDES        = @INCLIST@
AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns >/dev/null && pwd`:$$PATH ; \
  export top_builddir top_srcdir PATH
COLUMNS_EXE     = $(top_builddir)/columns/columns$(EXEEXT)


all : gen
gen : $(BUILT_SOURCES) $(DOCFILES)

opts.c $(getdefs_OBJECTS) : opts.h
gd.c : $(gdsrcs)
	( echo '#define DEFINE' ; \
	  for f in $(gdsrcs) ; do echo "#include \"$$f\"" ; done ) > $@

opts.h : opts.def $(COLUMNS_EXE) $(AGEXE)
	$(AG_ENV) ; $(AGEXE_LIB) $(srcdir)/opts.def

$(DOCFILES) : doc-stamp

$(AGEXE) :
	@cd ../agen5 ; $(MAKE) @AGEXE@

$(COLUMNS_EXE) :
	@cd ../columns ; $(MAKE) @COLEXE@

doc-stamp : opts.def $(COLUMNS_EXE) $(TARG)$(EXEEXT) $(AGEXE)
	$(AG_ENV) ; $(AGEXE_LIB) -Tagman1 -b$(TARG) $(srcdir)/opts.def ; \
	$(AGEXE_LIB) -Taginfo -b$(TARG) -DLEVEL=section $(srcdir)/opts.def
	@touch $@

# Makefile.am ends here
