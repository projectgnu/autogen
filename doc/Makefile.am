##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Fri Aug 15 12:04 1997
## Last Modified:   Wed Jun 30 09:12:51 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.15 1999/07/07 14:23:16 bkorb Exp $
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
AGARGS		= autogen=$(AGEXE) \
		agarg=--templ-dirs=$(top_srcdir) agarg=-t60 \
		base-name=autogen

autogen.texi : auto_gen.tpl $(DEFSRC) $(ODEF) $(TPLINC)
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	CC="${CC}" \
	$(GETDEFS) $(AGARGS) template=auto_gen $(DEFCOPY) $(DEFSRC)

www/doc/autogen_toc.html : autogen.texi
	@mkdir -f www/doc > /dev/null 2>&1
	cd www/doc ; texi2html -split_chapter -verbose \
		$(top_builddir)/doc/autogen.texi
	@touch $@

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
