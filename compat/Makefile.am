##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <korbb@datadesign.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    Mon Sep 14 06:59:22 1998
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.6 1998/09/14 14:33:58 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

EXTRA_DIST = @COMPAT_SRC@
GEN_SRC = strsignal.h
MAINTAINERCLEANFILES = Makefile.in $(GEN_SRC)

all:

strsignal.h : strsignal.def strsignal.tpl

# Makefile.am ends here
