##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Fri Aug 15 12:04 1997
## Last Modified:   Wed Aug  4 07:52:50 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.21 1999/08/04 14:53:03 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

# This variable is needed to subvert automake's info rules.
# They don't work for generated texi files:
#
INFO_DEPS	= autogen.info
MAKEINFO	= @MAKEINFO@ --no-split

all : $(INFO_DEPS) LSM

info_TEXINFOS   = autogen.texi

EXTRA_DIST	= auto_gen.tpl autogen.info LSM LSM.tpl depend.fig depend.eps
MAINTAINERCLEANFILES = Makefile.in autogen.texi

ODEF            = $(top_srcdir)/src/opts.def
SNVDOC          = $(top_srcdir)/snprintfv/doc/version.texi
DEFSRC		= \
		$(top_srcdir)/src/agFunCase.c \
		$(top_srcdir)/src/agFunEval.c \
		$(top_srcdir)/src/agFunFor.c \
		$(top_srcdir)/src/agFunIf.c \
		$(top_srcdir)/src/agFunOut.c \
		$(top_srcdir)/src/agFunc.c \
		$(top_srcdir)/src/agDirect.c \
		$(top_srcdir)/src/agExpr.c \
		$(top_srcdir)/autoopts/opthead.tpl

DEFCOPY		= copy=$(ODEF)

TEMPLATE	= $(srcdir)/auto_gen.tpl
TPLINC		= $(top_srcdir)/src/agParse.y \
		$(top_builddir)/src/opts.h \
		$(top_srcdir)/src/autogen.texi \
		$(top_srcdir)/autoopts/autoopts.texi \
		$(top_builddir)/columns/columns.texi \
		$(top_builddir)/getdefs/getdefs.texi

GETDEFS		= $(top_builddir)/getdefs/getdefs filelist
AGEXE		= $(top_builddir)/src/autogen
AGARGS		= base-name=autogen autogen=$(AGEXE) \
		agarg=-L$(srcdir) agarg=-L$(top_srcdir) agarg=-t60

autogen.texi : auto_gen.tpl $(DEFSRC) $(ODEF) $(TPLINC) $(SNVDOC)
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	CC="${CC}" \
	$(GETDEFS) $(AGARGS) template=auto_gen $(DEFCOPY) $(DEFSRC)

www/doc/autogen_toc.html : autogen.texi
	@mkdir -f www/doc > /dev/null 2>&1
	texi2html -menu -split_chapter -verbose autogen.texi
	mv -f *.html www/doc
	@touch $@

html : web
web : www/doc/autogen_toc.html

$(top_builddir)/getdefs/getdefs.texi :
	@cd $(top_builddir)/getdefs/ ; $(MAKE) gen
 
$(top_builddir)/columns/columns.texi :
	@cd $(top_builddir)/columns/ ; $(MAKE) gen

LSM : $(top_srcdir)/VERSION $(ODEF) LSM.tpl
	@if $(AGEXE) --version >&- 2>&- ; then \
	  echo $(AGEXE) -L $(srcdir) -T LSM.tpl $(ODEF) ; \
	  top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	    $(AGEXE) -L $(srcdir) -T LSM.tpl $(ODEF) ; \
	else \
	  if test -s $(srcdir)/$@; then \
	   echo "WARNING: $(AGEXE) not available, current $(srcdir)/$@ used"; \
	  else \
	   echo "ERROR: $(srcdir)/$@ has been corrupted"; \
	  fi; \
	fi

autogen.info : doc

doc :
	ln -s $(top_srcdir)/snprintfv/doc ./doc

# Makefile.am ends here
