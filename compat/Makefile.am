##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-11-01 08:40:35 bkorb"
## Copyright (c):   1997-2004 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997				      
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.6 2004/11/02 04:03:59 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

EXTRA_DIST = bootstrap.dir compat.h libgen.h pathfind.c strftime.c \
	strcspn.c strsignal.c strsignal.def strsignal.h strsignal.tpl

GEN_SRC    = strsignal.h
MAINTAINERCLEANFILES = Makefile.in

all:
	:

strsignal.h : strsignal.def strsignal.tpl
	@if $(AGexe) --version >&- 2>&- ; then \
	  echo $(AGexe) -L $(srcdir) $(srcdir)/strsignal.def ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGexe) -L $(srcdir) $(srcdir)/strsignal.def ; \
	elif $(AGnam) --version >&- 2>&- ; then \
	  echo $(AGnam) -L $(srcdir) $(srcdir)/strsignal.def ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGnam) -L $(srcdir) $(srcdir)/strsignal.def ; \
	else \
	  if test -s $(srcdir)/$@; then \
	   echo "WARNING: $(AGnam) not available, current $(srcdir)/$@ used"; \
	  else \
	   echo "ERROR: $(srcdir)/$@ has been corrupted"; exit 1;\
	  fi; \
	fi

# Makefile.am ends here
