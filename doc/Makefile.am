##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:  $Date: 1998/06/01 20:57:46 $
##            by:  Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.4 1998/06/01 20:57:46 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

all : autogen.info

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

EXTRA_DIST	= autogen.def auto_gen.tpl autogen.texisav
MAINTAINERCLEANFILES = Makefile.in

ODEF		= $(top_builddir)/src/opts.def
info_TEXINFOS	= autogen.texi
BUILT_SOURCES	= autogen.texi

gendir	= $(top_builddir)/src
docdir	= $(top_srcdir)/doc
optdir	= $(top_srcdir)/autoopts

AGEN	= $(gendir)/autogen
ODEF	= $(gendir)/opts.def
DIRECT  = $(top_builddir)/src/agdirect.def
EXPR    = $(top_builddir)/src/agexpr.def
FUNC    = $(top_builddir)/src/agfunc.def

autogen.texi: auto_gen.tpl autogen.def $(ODEF) $(DIRECT) $(EXPR) $(FUNC)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
		$(AGEN) autogen.def

autogen.info : autogen.texi

# Makefile.am ends here
