##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:   Mon Sep 28 12:07:41 1998
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.6 1998/09/28 19:33:05 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS     = autogen.info

all : $(INFO_DEPS) LSM

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl LSM LSM.tpl
MAINTAINERCLEANFILES = Makefile.in autogen.texi

ODEF            = $(top_srcdir)/src/opts.def

DEFSRC		= $(top_srcdir)/src/agFun*.c \
		$(top_srcdir)/src/agDirect.c \
		$(top_srcdir)/src/agExpr.c \
		$(top_srcdir)/autoopts/opthead.tpl

DEFCOPY		= copy=$(ODEF)

TEMPLATE	= $(srcdir)/auto_gen.tpl
TPLINC		= $(top_srcdir)/src/agParse.y \
		$(top_builddir)/src/opts.h \
		$(top_srcdir)/autoopts/autoopts.texi \
		$(top_builddir)/columns/columns.texi \
		$(top_builddir)/getdefs/getdefs.texi

GETDEFS		= $(top_builddir)/getdefs/getdefs filelist
AGEXE		= $(top_builddir)/src/autogen
AGARGS		= autogen=$(AGEXE) \
		agarg=--templ-dirs=$(top_srcdir) \
		base-name=autogen

autogen.def:	$(DEFSRC) $(ODEF)
	$(GETDEFS) output=$@ template=auto_gen $(DEFCOPY) $(DEFSRC)
	@-touch $@

##  This really should be the build rule:
##	$(GETDEFS) $(AGARGS) template=auto_gen $(DEFCOPY) $(DEFSRC)
##
autogen.texi : auto_gen.tpl autogen.def $(TPLINC)
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -L$(top_srcdir) -L$(top_srcdir)/doc \
		-bautogen ./autogen.def

$(top_builddir)/getdefs/getdefs.texi :
	@cd $(top_builddir)/getdefs/ ; $(MAKE) gen
 
$(top_builddir)/columns/columns.texi :
	@cd $(top_builddir)/columns/ ; $(MAKE) gen

LSM : $(top_srcdir)/VERSION $(ODEF) LSM.tpl
	@if $(AGEXE) --version >&- 2>&- ; then \
	  echo $(AGEXE) -L $(srcdir) -T LSM.tpl $(ODEF) ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGEXE) -L $(srcdir) -T LSM.tpl $(ODEF) ; \
	else \
	  if test -s $(srcdir)/$@; then \
	   echo "WARNING: $(AGEXE) not available, current $(srcdir)/$@ used"; \
	  else \
	   echo "ERROR: $(srcdir)/$@ has been corrupted"; \
	  fi; \
	fi

# Makefile.am ends here
