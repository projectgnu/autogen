##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2006-01-22 11:03:42 bkorb"
## Copyright (c):   1997-2006 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri Aug 15 12:04 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.8 2006/03/25 19:23:28 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS       = autogen.info
MIexe           = $(MAKEINFO) --no-split
MAKEINFOFLAGS   = -I$(top_srcdir)/autoopts -I../autoopts

all : $(INFO_DEPS)

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl snprintfv.texi snprintfv.menu \
		mk-agen-texi-in

MAINTAINERCLEANFILES  = MakeDep.inc
TEXI2DVI_FLAGS  =  --texinfo='@pagesizes 9.5in,7.0in'

autogen.texi : # self-depends upon all executables
	MAKE="$(MAKE)" ./mk-agen-texi.sh

# Special rule for generating all the GNU standard formats.
#
autogen.txt : autogen.texi
	$(MIexe) --fill-column=70 --paragraph-indent=0 --no-headers \
		--output=$@ autogen.texi

clobber : maintainer-clean

# Makefile.am ends here
