##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:  $Date: 1998/07/14 13:35:13 $
##            by:  Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.8 1998/07/14 13:35:13 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS     = autogen.info

all : $(INFO_DEPS)

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= autogen.def auto_gen.tpl
MAINTAINERCLEANFILES = Makefile.in autogen.texi

gendir	= $(top_builddir)/src
optdir  = $(top_builddir)/src

AODEFS  = $(top_srcdir)/autoopts/options.tpl
AOHEAD  = $(top_srcdir)/autoopts/opthead.tpl
ODEF            = $(optdir)/opts.def
DIRECT          = $(gendir)/agdirect.def
EXPR            = $(gendir)/agexpr.def
FUNC            = $(gendir)/agfunc.def
AGINCLUDE       = $(ODEF) $(DIRECT) $(EXPR) $(FUNC) $(AODEFS) $(AOHEAD)

autogen.texi : auto_gen.tpl autogen.def $(AGINCLUDE)

# Makefile.am ends here
