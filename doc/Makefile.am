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
## $Id: Makefile.am,v 2.40 2000/04/04 13:23:05 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS	= autogen.info
MAKEINFO	= @MAKEINFO@ --no-split

all : $(INFO_DEPS) LSM

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl autogen.info LSM LSM.tpl depend.fig depend.eps \
		snprintfv.texi snprintfv.menu

MAINTAINERCLEANFILES = Makefile.in autogen.texi

AGSRCDIR	= $(top_srcdir)/agen5

OPTIONS_DEF	= $(AGSRCDIR)/opts.def

FUNCSRC		= \
		$(AGSRCDIR)/funcCase.c \
		$(AGSRCDIR)/funcDef.c \
		$(AGSRCDIR)/funcEval.c \
		$(AGSRCDIR)/funcFor.c \
		$(AGSRCDIR)/funcIf.c \
		$(AGSRCDIR)/functions.c

EXPRSRC		= \
		$(AGSRCDIR)/expFormat.c \
		$(AGSRCDIR)/expGuile.c \
		$(AGSRCDIR)/expOutput.c \
		$(AGSRCDIR)/expPrint.c \
		$(AGSRCDIR)/expState.c \
		$(AGSRCDIR)/expString.c \
		$(FUNCSRC)

DIRECTIVESRC	= $(AGSRCDIR)/defDirect.c

GETDEF_SRC	= $(EXPRSRC) $(DIRECTIVESRC)

ADDON_MENU	= \
		$(top_srcdir)/columns/columns.menu \
		$(top_srcdir)/getdefs/getdefs.menu \
		$(srcdir)/snprintfv.menu

ADDON_TEXI	= \
		$(top_srcdir)/columns/columns.texi \
		$(top_srcdir)/getdefs/getdefs.texi \
		$(srcdir)/snprintfv.texi

DOC_INCLUDES	= $(AGSRCDIR)/defParse.y \
		$(AGSRCDIR)/opts.h \
		$(AGSRCDIR)/autogen.menu \
		$(AGSRCDIR)/autogen.texi \
		$(top_srcdir)/autoopts/doc.tpl

DOC_TEMPLATE	= $(srcdir)/auto_gen.tpl

DOC_DEPENDS	= $(DOC_TEMPLATE) \
		$(OPTIONS_DEF) \
		$(GETDEF_SRC) \
		$(ADDON_MENU) \
		$(ADDON_TEXI) \
		$(DOC_INCLUDES)

DEFCOPY		= copy=$(OPTIONS_DEF)

GET_OPTS	= srcfile linenum filelist template=auto_gen
BLOCKOPT	= subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
GETDEFS		= $(top_builddir)/getdefs/getdefs $(GET_OPTS) $(BLOCKOPT)

AGEXE		= `cd $(top_builddir)/agen5 ; pwd`/@AGEXE@

AGARGS		= base-name=autogen autogen=$(AGEXE) \
		agarg="-L$(srcdir) -L$(top_srcdir) -t60"

autogen.texi : $(DOC_DEPENDS)
	@echo $(GETDEFS) $(AGARGS) $(DEFCOPY) ...
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AGEXE=$(AGEXE) CC="$(CC)" \
	ADDON_MENU="$(ADDON_MENU)" \
	ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS)" \
	$(GETDEFS) $(AGARGS) $(DEFCOPY) $(GETDEF_SRC)

##  The following rule is for debugging generation problems:
##
autogen.def : $(DOC_DEPENDS)
	@echo $(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	ADDON_MENU="$(ADDON_MENU)" \
	ADDON_TEXI="$(ADDON_TEXI)" \
	DOC_DEPENDS="$(DOC_DEPENDS)" \
	$(GETDEFS) output=$@ $(DEFCOPY) $(GETDEF_SRC)

$(AGSRCDIR)/autogen.texi :
	@cd $(AGSRCDIR) ; echo making $@ in `pwd` ; make autogen.texi

www/doc/autogen_toc.html : autogen.texi
	@mkdir -p www/doc > /dev/null 2>&1 || :
	texi2html -menu -split_chapter -verbose autogen.texi
	mv -f autogen*.html www/doc
	@touch $@

html : web
web : www/doc/autogen_toc.html

$(top_srcdir)/getdefs/getdefs.texi :
	@cd $(top_srcdir)/getdefs/ ; $(MAKE) -f ./Makefile gen
 
$(top_srcdir)/columns/columns.texi :
	@cd $(top_srcdir)/columns/ ; $(MAKE) -f ./Makefile gen

LSM : $(top_srcdir)/VERSION $(ODEF) LSM.tpl
	top_builddir=$(top_builddir) \
	$(AGEXE) -T $(srcdir)/LSM.tpl $(OPTIONS_DEF)

clobber : maintainer-clean

# Makefile.am ends here
