##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:  $Date: 1998/06/22 03:10:18 $
##            by:  Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.6 1998/06/22 03:10:18 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS     = autogen.info

all : $(INFO_DEPS)

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

EXTRA_DIST      = autogen.def auto_gen.tpl autogen.texisav
MAINTAINERCLEANFILES = Makefile.in

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= autogen.def auto_gen.tpl
MAINTAINERCLEANFILES = Makefile.in

gendir	= $(top_builddir)/src
optdir  = $(top_builddir)/src

ODEF            = $(optdir)/opts.def
DIRECT          = $(gendir)/agdirect.def
EXPR            = $(gendir)/agexpr.def
FUNC            = $(gendir)/agfunc.def
AGINCLUDE       = $(ODEF) $(DIRECT) $(EXPR) $(FUNC)

autogen.texi : auto_gen.tpl autogen.def $(AGINCLUDE)

# Makefile.am ends here
