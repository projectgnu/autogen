##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:         Bruce Korb <korbb@datadesign.com>
## Maintainer:     Bruce Korb <korbb@datadesign.com>
## Created:        Fri Aug 15 12:04 1997
## Last Modified:  1998/03/10 at 10:08:25
##            by:  Bruce Korb <korbb@datadesign.com>

all : autogen.info

info_TEXINFOS	= autogen.texi
# EXTRA_DIST       = autogen_toc.html autogen.html
EXTRA_DIST	= autogen.def auto_gen.tpl

gendir	= $(top_builddir)/src
docdir	= $(top_srcdir)/doc
optdir	= $(top_srcdir)/autoopts

AGEN	= $(gendir)/autogen
ODEF	= $(gendir)/opts.def
DIRECT  = $(top_builddir)/src/agdirect.def
EXPR    = $(top_builddir)/src/agexpr.def

autogen.texi: auto_gen.tpl autogen.def $(ODEF) $(DIRECT) $(EXPR)
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
		$(AGEN) autogen.def

autogen.info : autogen.texi

# Makefile.am ends here
