##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Fri Aug 15 12:04 1997
## Last Modified:   Tue Oct 20 15:09:59 1998
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.9 1998/10/30 16:49:16 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS     = autogen.info

all : $(INFO_DEPS) LSM

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl LSM LSM.tpl depend.fig depend.eps
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

autogen.texi : auto_gen.tpl $(DEFSRC) $(ODEF) $(TPLINC)
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	CC="${CC}" \
	$(GETDEFS) $(AGARGS) template=auto_gen $(DEFCOPY) $(DEFSRC)

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
