##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:   Tue Sep 15 14:55:24 1998
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.2 1998/09/22 21:26:00 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS     = autogen.info

all : $(INFO_DEPS) LSM

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@
DOCDIRS         = -L$(top_srcdir)

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= autogen.def auto_gen.tpl LSM LSM.tpl
MAINTAINERCLEANFILES = Makefile.in autogen.texi

gendir          = $(top_builddir)/src
optdir          = $(top_srcdir)/src

AODEFS          = $(top_srcdir)/autoopts/options.tpl
AOHEAD          = $(top_srcdir)/autoopts/opthead.tpl
ODEF            = $(optdir)/opts.def
DIRECT          = $(gendir)/agdirect.def
EXPR            = $(gendir)/agexpr.def
FUNC            = $(gendir)/agfunc.def
AGDEPS          = $(ODEF) $(DIRECT) $(EXPR) $(FUNC) $(AODEFS) $(AOHEAD) \
		$(top_srcdir)/src/agParse.y \
		$(top_srcdir)/autoopts/autoopts.texi \
		$(top_srcdir)/getdefs/getdefs.texi

autogen.texi : auto_gen.tpl autogen.def $(AGDEPS)

LSM : $(top_srcdir)/VERSION $(top_srcdir)/src/opts.def LSM.tpl
	@if $(AGEXE) --version >&- 2>&- ; then \
	  echo $(AGEXE) -T LSM.tpl $(ODEF) ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGEXE) -T LSM.tpl $(ODEF) ; \
	else \
	  if test -s $(srcdir)/$@; then \
	   echo "WARNING: $(AGEXE) not available, current $(srcdir)/$@ used"; \
	  else \
	   echo "ERROR: $(srcdir)/$@ has been corrupted"; \
	  fi; \
	fi

# Makefile.am ends here
