##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri Aug 15 12:04 1997
## Last Modified:   Thu Aug 12 17:19:38 1999
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.16 2003/08/24 00:01:30 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS	= autogen.info
MAKEINFO	= @MAKEINFO@ --no-split
MAKEINFOFLAGS = -I$(top_srcdir)/autoopts -I../autoopts

all : $(INFO_DEPS)

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl autogen.info snprintfv.texi snprintfv.menu

MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = texi-stamp

AGSRCDIR	= $(top_srcdir)/agen5

OPTIONS_DEF	= $(AGSRCDIR)/opts.def

DIRECTIVESRC	= $(AGSRCDIR)/defDirect.c

GETDEF_SRC	= $(AGSRCDIR)/func*.c $(AGSRCDIR)/exp*.c $(DIRECTIVESRC)

ADDON_MENU	= \
		../columns/columns.menu \
		../getdefs/getdefs.menu \
		../xml2ag/xml2ag.menu \
		$(srcdir)/snprintfv.menu

ADDON_TEXI	= \
		../columns/columns.texi \
		../getdefs/getdefs.texi \
		../xml2ag/xml2ag.texi \
		$(srcdir)/snprintfv.texi

DOC_INCLUDES	= $(AGSRCDIR)/defParse.y \
		$(AGSRCDIR)/opts.h \
		../agen5/autogen.menu \
		../autoopts/libopts.texi \
		../agen5/autogen.texi

DOC_TEMPLATE	= $(srcdir)/auto_gen.tpl

DOC_DEPENDS	= $(DOC_TEMPLATE) \
		$(OPTIONS_DEF) \
		$(ADDON_MENU) \
		$(ADDON_TEXI) \
		$(DOC_INCLUDES)

DEFCOPY		= copy=$(OPTIONS_DEF)

GET_OPTS	= srcfile linenum filelist template=$(DOC_TEMPLATE)
BLOCKOPT	= subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
GETDEFS		= $(top_builddir)/getdefs/getdefs $(GET_OPTS) $(BLOCKOPT)

AGEXE		= `cd $(top_builddir)/agen5   ; pwd`/@AGEXE@
COLUMNS_EXE = `cd $(top_builddir)/columns ; pwd`/columns@EXEEXT@

TEXI2DVI_FLAGS =  --texinfo='@pagesizes 9.5in,7.0in'

texi         : texi-stamp
autogen.texi : texi

texi-stamp : autogen.def
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AGEXE=$(AGEXE) CC="$(CC)" COLUMNS_EXE="$(COLUMNS_EXE)" \
	ADDON_MENU="$(ADDON_MENU)" ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS) `echo $(GETDEF_SRC)`" \
	LIBS="$(LIBS)" SHELL=/bin/sh \
	$(AGEXE) -t180 -L$(srcdir) -L$(top_srcdir) autogen.def
	@touch $@ ; chmod u+w $(srcdir) . || :

autogen.def : $(DOC_DEPENDS)
	@echo $(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	$(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)

##  During the product build, the following .texi and .menu files may not
##  have been built 'cuz the needed programs are not yet in place.
##  Do them now.
##
../agen5/autogen.menu : ../agen5/autogen.texi
../agen5/autogen.texi :
	cd ../agen5 && $(MAKE) autogen.texi

../getdefs/getdefs.menu : ../getdefs/getdefs.texi
../getdefs/getdefs.texi :
	@cd ../getdefs/ ; $(MAKE) doc-stamp

../columns/columns.menu : ../columns/columns.texi
../columns/columns.texi :
	@cd ../columns/ ; $(MAKE) gen-stamp

../xml2ag/xml2ag.menu : ../xml2ag/xml2ag.texi
../xml2ag/xml2ag.texi :
	@if [ -z "$(XML2AG_DIR)" ] ; \
	then [ -d ../xml2ag ] || mkdir ../xml2ag ; \
	echo "* xml2ag Invocation::             No invoking xml2ag" \
	     > ../xml2ag/xml2ag.menu ; \
	( echo "@node xml2ag Invocation" ; \
	  echo "@section No invoking xml2ag" ; \
	  echo "This program is not available without libxml2." \
	) > ../xml2ag/xml2ag.texi ; \
	else cd ../xml2ag ; $(MAKE) doc-stamp ; fi

../autoopts/libopts.texi :
	@cd ../autoopts ; $(MAKE) libopts.texi

autogen.txt : autogen.texi
	makeinfo --fill-column=70 --no-split --paragraph-indent=0 \
	         --no-headers --output=$@ autogen.texi

clobber : maintainer-clean

# Makefile.am ends here
