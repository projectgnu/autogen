##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri Aug 15 12:04 1997
## Last Modified:   Thu Aug 12 17:19:38 1999				      
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.1 2001/12/29 11:24:56 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS	= autogen.info
MAKEINFO	= @MAKEINFO@ --no-split

all : $(INFO_DEPS) autogen.lsm

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl autogen.info LSM.tpl \
		snprintfv.texi snprintfv.menu compete.def compete.tpl

MAINTAINERCLEANFILES = Makefile.in autogen.texi

AGSRCDIR	= $(top_srcdir)/agen5

OPTIONS_DEF	= $(AGSRCDIR)/opts.def

DIRECTIVESRC	= $(AGSRCDIR)/defDirect.c

GETDEF_SRC	= $(AGSRCDIR)/func*.c $(AGSRCDIR)/exp*.c $(DIRECTIVESRC)

ADDON_MENU	= \
		../columns/columns.menu \
		../getdefs/getdefs.menu \
		$(srcdir)/snprintfv.menu

ADDON_TEXI	= \
		../columns/columns.texi \
		../getdefs/getdefs.texi \
		$(srcdir)/snprintfv.texi

DOC_INCLUDES	= $(AGSRCDIR)/defParse.y \
		$(AGSRCDIR)/opts.h \
		$(AGSRCDIR)/autogen.menu \
		$(AGSRCDIR)/autogen.texi

DOC_TEMPLATE	= $(srcdir)/auto_gen.tpl

DOC_DEPENDS	= $(DOC_TEMPLATE) \
		$(OPTIONS_DEF) \
		$(ADDON_MENU) \
		$(ADDON_TEXI) \
		$(DOC_INCLUDES) compete.def compete.tpl

DEFCOPY		= copy=$(OPTIONS_DEF)

GET_OPTS	= srcfile linenum filelist template=$(DOC_TEMPLATE)
BLOCKOPT	= subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
GETDEFS		= $(top_builddir)/getdefs/getdefs $(GET_OPTS) $(BLOCKOPT)

AGEXE		= `cd $(top_builddir)/agen5 ; pwd`/@AGEXE@

AGARGS		= base-name=autogen autogen=$(AGEXE) \
		agarg="-L$(srcdir) -L$(top_srcdir) -t60"

TEXI2DVI_FLAGS =  --texinfo='@pagesizes 9.5in,7.0in'

texi : texi-stamp

texi-stamp : $(DOC_DEPENDS)
	@echo $(GETDEFS) $(AGARGS) $(DEFCOPY) ... ; \
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AGEXE=$(AGEXE) CC="$(CC)" \
	ADDON_MENU="$(ADDON_MENU)" \
	ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS) `echo $(GETDEF_SRC)`" \
	$(GETDEFS) $(AGARGS) $(DEFCOPY) $(GETDEF_SRC)
	@touch $@ ; if [ -f autogen.info ] ; then chmod u+w autogen.info || : ; fi

##  The following rule is for debugging generation problems:
##
autogen.def : $(DOC_DEPENDS)
	@echo $(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	ADDON_MENU="$(ADDON_MENU)" \
	ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS)" \
	$(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)
	@touch $@

www/doc/autogen_toc.html : texi-stamp
	@mkdir -p www/doc > /dev/null 2>&1 || :
	texi2html -menu -split_chapter -verbose autogen.texi
	mv -f autogen*.html www/doc
	@-sleep 1 ; touch $@

html : web
web : www/doc/autogen_toc.html

../getdefs/getdefs.texi :
	@cd ../getdefs/ ; $(MAKE) $$mf gen
 
../columns/columns.texi :
	@cd ../columns/ ; $(MAKE) $$mf gen

autogen.lsm : $(top_srcdir)/VERSION $(ODEF) LSM.tpl
	top_builddir=$(top_builddir) top_srcdir=$(top_srcdir) \
	$(AGEXE) -T $(srcdir)/LSM.tpl $(OPTIONS_DEF)

clobber : maintainer-clean

# Makefile.am ends here
