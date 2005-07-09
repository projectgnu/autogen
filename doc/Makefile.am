##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-07-09 15:16:35 bkorb"
## Copyright (c):   1997-2005 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri Aug 15 12:04 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.4 2005/07/09 22:54:51 bkorb Exp $
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
		mk-libopts-texi-in mk-agen-texi-in

DISTCLEANFILES  = texi-stamp
TEXI2DVI_FLAGS  =  --texinfo='@pagesizes 9.5in,7.0in'

texi         :
	@:

autogen.texi : texi-stamp

#  This dependency can only be satisfied if all executables exist.
#  The reconstruction of the .texi file requires running these commands
#  with the ``--help'' option in order to get the usage text into the doc.
#
texi-stamp : $(AGexe) $(CLexe) $(GDexe)
	MAKE="$(MAKE)" ./mk-agen-texi.sh

$(AGexe) :
	@cd ../agen5 && $(MAKE) $(AGnam)

$(GDexe) :
	@cd ../getdefs && $(MAKE) $(AGnam)

$(CLexe) :
	@cd ../columns && $(MAKE) $(CLnam)

# Special rule for generating all the GNU standard formats.
#
autogen.txt : autogen.texi
	$(MIexe) --fill-column=70 --paragraph-indent=0 --no-headers \
		--output=$@ autogen.texi

clobber : maintainer-clean

# Makefile.am ends here
