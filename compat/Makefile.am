##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <korbb@datadesign.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    $Date: 1998/08/17 14:19:12 $
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.5 1998/08/17 14:19:12 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

EXTRA_DIST = @COMPAT_SRC@
BUILT_SOURCES = strsignal.h
MAINTAINERCLEANFILES = Makefile.in $(BUILT_SOURCES)

all:

strsignal.h : strsignal.def strsignal.tpl

# Makefile.am ends here
