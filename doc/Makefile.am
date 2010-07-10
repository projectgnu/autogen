##  -*- Mode: Makefile -*-
##
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2010-07-10 16:45:57 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
##
## This file is part of AutoGen.
## AutoGen Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
## ---------------------------------------------------------------------

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS       = autogen.info
MIexe           = $(MAKEINFO) --no-split
MAKEINFOFLAGS   = -I$(top_srcdir)/autoopts -I../autoopts

all : $(INFO_DEPS)

BUILT_SOURCES   = autogen.texi
info_TEXINFOS   = $(BUILT_SOURCES)

EXTRA_DIST	= auto_gen.tpl snprintfv.texi snprintfv.menu \
		mk-agen-texi-in autogen-texi.txt auto-opts.tpl

MAINTAINERCLEANFILES  = MakeDep.inc
TEXI2DVI_FLAGS  =  --texinfo='@pagesizes 9.5in,7.0in'

agdoc.texi      : # self-depends upon all executables
	MAKE="$(MAKE)" ./mk-agen-texi.sh

autogen.dvi     : agdoc.texi
autogen.texi    : agdoc.texi

# Special rule for generating all the GNU standard formats.
#
autogen.txt : autogen.texi
	$(MIexe) --fill-column=70 --paragraph-indent=0 --no-headers \
		--output=$@ autogen.texi

clobber : maintainer-clean

.NOTPARALLEL:

# Makefile.am ends here
