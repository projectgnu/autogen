##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 2003 Bruce Korb
## 
## Author:           Bruce Korb <bkorb@gnu.org>
## Maintainer:       Bruce Korb <bkorb@gnu.org>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    Thu Jul 29 08:30:10 1999				      
##            by:    Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.2 2003/02/16 00:04:40 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

EXTRA_DIST = bootstrap.dir compat.h libgen.h ndir.h pathfind.c strftime.c \
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
