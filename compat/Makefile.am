##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-02-01 13:24:27 bkorb"
## Copyright (c):   1997-2004 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997				      
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.4 2004/02/01 21:26:46 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

EXTRA_DIST = bootstrap.dir compat.h libgen.h pathfind.c strftime.c \
	strcspn.c strsignal.c strsignal.def strsignal.h strsignal.tpl

GEN_SRC    = strsignal.h
MAINTAINERCLEANFILES = Makefile.in
AGEXE      = $(top_builddir)/src/autogen

all:
	:

strsignal.h : strsignal.def strsignal.tpl
	@if $(AGEXE) --version >&- 2>&- ; then \
	  echo $(AGEXE) -L $(srcdir) $(srcdir)/strsignal.def ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGEXE) -L $(srcdir) $(srcdir)/strsignal.def ; \
	else \
	  if test -s $(srcdir)/$@; then \
	   echo "WARNING: $(AGEXE) not available, current $(srcdir)/$@ used"; \
	  else \
	   echo "ERROR: $(srcdir)/$@ has been corrupted"; \
	  fi; \
	fi

# Makefile.am ends here
