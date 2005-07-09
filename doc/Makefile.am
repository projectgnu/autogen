##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-07-09 09:57:02 bkorb"
## Copyright (c):   1997-2005 by Bruce Korb
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri Aug 15 12:04 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.3 2005/07/09 17:29:50 bkorb Exp $
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

EXTRA_DIST	= auto_gen.tpl autogen.info snprintfv.texi snprintfv.menu \
		mk-libopts-texi-in

MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES  = texi-stamp autogen.def

AGSRCDIR	= $(top_srcdir)/agen5
OPTIONS_DEF	= $(AGSRCDIR)/opts.def
DIRECTIVESRC	= $(AGSRCDIR)/defDirect.c
GETDEF_SRC	= $(AGSRCDIR)/func*.c $(AGSRCDIR)/exp*.c $(DIRECTIVESRC)

AG_ENV          = top_builddir=$(top_builddir) \
  top_srcdir=$(top_srcdir) PATH=`cd ../columns;pwd`:$$PATH \
  AGexe=$(AGexe) CLexe=$(CLexe) GDexe=$(GDexe) ; \
  export top_builddir top_srcdir PATH AGexe CLexe GDexe

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

DOC_INCLUDES	= $(AGSRCDIR)/defParse-fsm.c \
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

DEFCOPY         = copy=$(OPTIONS_DEF)
BLOCKOPT        = subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
TEXI2DVI_FLAGS  =  --texinfo='@pagesizes 9.5in,7.0in'

texi         : texi-stamp
autogen.texi : texi

texi-stamp : autogen.def $(AGexe) $(CLexe)
	$(AG_ENV) ; \
	CC="$(CC)" CFLAGS='$(CFLAGS)' \
	ADDON_MENU="$(ADDON_MENU)" ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS) `echo $(GETDEF_SRC)`" \
	LIBS="$(LIBS)" SHELL=/bin/sh MAKE='$(MAKE)' \
	$(AGexe) -t180 -L$(srcdir) -L$(top_srcdir) autogen.def
	@touch $@ ; chmod u+w $(srcdir) . || :

autogen.def : $(DOC_DEPENDS) $(GDexe)
	@( echo output   $@ ; \
	   echo copy     $(OPTIONS_DEF) ; \
	   echo srcfile  ; echo linenum ; \
	   echo template auto_gen.tpl ; \
	   for f in $(GETDEF_SRC) ; do \
	     echo input $$f ; done ) > getdefs.ini ; \
	echo + $(GDexe) load-opt=getdefs.ini
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	$(GDexe) load-opt=getdefs.ini || { cat getdefs.ini ; exit 1 ; }

##  During the product build, the following .texi and .menu files may not
##  have been built 'cuz the needed programs are not yet in place.
##  Do them now.
##
$(AGexe) :
	@cd ../agen5 && $(MAKE) $(AGnam)

$(GDexe) :
	@cd ../getdefs && $(MAKE) $(AGnam)

$(CLexe) :
	@cd ../columns && $(MAKE) $(CLnam)

../agen5/autogen.menu : ../agen5/autogen.texi
../agen5/autogen.texi :
	cd ../agen5 && $(MAKE) autogen.texi

../getdefs/getdefs.menu : ../getdefs/getdefs.texi
../getdefs/getdefs.texi :
	@cd ../getdefs ; $(MAKE) doc-stamp

../columns/columns.menu : ../columns/columns.texi
../columns/columns.texi :
	@cd ../columns ; $(MAKE) gen-stamp

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
	$(MIexe) --fill-column=70 --paragraph-indent=0 --no-headers \
		--output=$@ autogen.texi

clobber : maintainer-clean

# Makefile.am ends here
