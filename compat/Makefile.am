##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <korbb@datadesign.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    Mon Sep 14 06:59:22 1998
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.0 1998/09/22 22:17:27 bkorb Exp $
## ---------------------------------------------------------------------
## Code:


EXTRA_DIST = @COMPAT_SRC@
GEN_SRC    = strsignal.h
MAINTAINERCLEANFILES = Makefile.in $(GEN_SRC)
AGEXE      = $(top_builddir)/src/autogen

all:

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
