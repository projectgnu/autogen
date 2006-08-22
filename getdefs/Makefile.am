##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright 1992-2006 Bruce Korb
##
## Time-stamp:      "2006-08-22 07:17:40 bkorb"
## Copyright (c):   1997-2006 by Bruce Korb
## Author:	        Bruce Korb <bkorb@gnu.org>
## Created:	        Mon Jun 30, 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.13 2006/08/22 16:06:36 bkorb Exp $
## ---------------------------------------------------------------------
TARG = getdefs

bin_PROGRAMS    = getdefs
gdsrcs          = getdefs.h proto.h getdefs.c gdemit.c gdinit.c opts.c
getdefs_SOURCES = proto.h
nodist_getdefs_SOURCES = gd.c opts.h

SUBDIRS         = test
BUILT_SOURCES   = opts.c opts.h gd.c
EXTRA_DIST      = opts.def $(gdsrcs)
## RUNAG        = valgrind $(AGexe)
RUNAG           = $(AGexe)
AGEXE_LIB       = $(RUNAG) -L$(top_srcdir)/autoopts
getdefs_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = $(TARG).1
DOCFILES        = $(TARG).texi $(TARG).menu $(TARG).1
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = $(DOCFILES) $(nodist_getdefs_SOURCES) *-stamp
INCLUDES        = @INCLIST@
AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns >/dev/null && pwd`:$$PATH ; \
  export top_builddir top_srcdir PATH


all : gen
gen : $(BUILT_SOURCES) $(DOCFILES)

opts.c $(getdefs_OBJECTS) : opts.h
gd.c : $(gdsrcs)
	( echo '#define DEFINE' ; \
	  for f in $(gdsrcs) ; do echo "#include \"$$f\"" ; done ) > $@

opts.h : opts.def $(CLexe) $(AGexe)
	$(AG_ENV) ; $(AGEXE_LIB) $(srcdir)/opts.def

$(DOCFILES) : doc-stamp

$(AGexe) :
	@cd ../agen5 ; $(MAKE) $(AGnam)

$(CLexe) :
	@cd ../columns ; $(MAKE) $(CLnam)

doc-stamp : opts.def $(CLexe) $(TARG)$(EXEEXT) $(AGexe)
	$(AG_ENV) ; $(AGEXE_LIB) -b$(TARG) -Tagman1 $(srcdir)/opts.def ; \
	$(AGEXE_LIB) -b$(TARG) -Taginfo -DLEVEL=section $(srcdir)/opts.def
	@touch $@

# Makefile.am ends here
