##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <autogen@linuxbox.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    Thu Jul 29 08:30:10 1999				      
##            by:    Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.8 1999/10/28 03:36:29 bruce Exp $
## ---------------------------------------------------------------------
## Code:

EXTRA_DIST	= bootstrap.dir compat.h gnu-regex.c gnu-regex.h \
		libgen.h ndir.h pathfind.c sigsend.c strftime.c \
		strsignal.c strsignal.def strsignal.h strsignal.tpl

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
