##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
## Copyright (C) 1997 Gary V Vaughan
## 
## Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>	      
## Maintainer:       Bruce Korb <korbb@datadesign.com>
## Created:          Mon Jun 30 15:35:12 1997	
## Last Modified:    1998/04/01 at 08:58:01
##            by:    Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.1.1.1 1998/04/29 23:14:31 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

EXTRA_DIST = @COMPAT_SRC@

all : strsignal.h

strsignal.h : strsignal.def strsignal.tpl /usr/include/sys/signal.h
	@-rm -f $@
	if ( ${top_builddir}/src/autogen -v > /dev/null 2>&1 ) ; then \
		${top_builddir}/src/autogen strsignal.def ; \
	else cp -p $@sav $@ ; fi

# Makefile.am ends here
