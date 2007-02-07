##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2007-01-27 06:55:28 bkorb"
## Copyright (c):   1997-2007 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.9 2007/02/07 01:57:58 bkorb Exp $
## ---------------------------------------------------------------------
## Code:
COMPAT_CSRC = pathfind.c snprintf.c strchr.c strdup.c strftime.c strsignal.c
COMPAT_HDRS = compat.h strsignal.h windows-config.h
COMPAT_GEN  = bootstrap.dir strsignal.def strsignal.tpl
EXTRA_DIST  = $(COMPAT_CSRC) $(COMPAT_HDRS) $(COMPAT_GEN)

MAINTAINERCLEANFILES = Makefile.in strsignal.h

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
	elif test -s $(srcdir)/$@; then \
	  echo "WARNING: $(AGnam) not available, current $(srcdir)/$@ used"; \
	else \
	  echo "ERROR: $(srcdir)/$@ has been corrupted"; exit 1;\
	fi

.NOTPARALLEL:

# Makefile.am ends here
