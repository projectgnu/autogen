##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:  $Date: 1998/06/02 15:26:20 $
##            by:  Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.5 1998/06/02 15:26:20 bkorb Exp $
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
BUILT_SOURCES   = autogen.texi

ODEF            = $(top_builddir)/src/opts.def
DIRECT          = $(top_builddir)/src/agdirect.def
EXPR            = $(top_builddir)/src/agexpr.def
FUNC            = $(top_builddir)/src/agfunc.def
AGINCLUDE       = $(ODEF) $(DIRECT) $(EXPR) $(FUNC)

autogen.texi : auto_gen.tpl autogen.def $(AGINCLUDE)

autogen.info : autogen.texi

# Makefile.am ends here
