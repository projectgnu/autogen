##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <korbb@datadesign.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    Tue Apr 13 08:23:43 1999
##            by:    Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.2 1999/04/14 17:44:04 bkorb Exp $
## ---------------------------------------------------------------------
## Code:


EXTRA_DIST = @COMPAT_SRC@
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
