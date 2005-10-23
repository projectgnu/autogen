##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-10-22 16:32:27 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.3 2005/10/23 02:40:02 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

if DO_SHELL_CMDS
if  HAVE_XML_LIB
SUBDIRS     = compat snprintfv autoopts agen5 columns getdefs xml2ag doc pkg
else
SUBDIRS     = compat snprintfv autoopts agen5 columns getdefs doc pkg
endif
else

## Without shell commands, you cannot build docs or packages and the
## columns and getdefs commands are unworkable.
##
if  HAVE_XML_LIB
SUBDIRS     = compat snprintfv autoopts agen5 xml2ag
else
SUBDIRS     = compat snprintfv autoopts agen5
endif
endif

cfg_gen        = config/ag_macros.m4 config/snprintfv.m4
config_dist    = config/missing config/misc.def config/libopts.def $(cfg_gen)
pkgdata_DATA   = config/conftest.tpl config/confmacs.tpl
pkg_dist       = config/bootstrap.local
EXTRA_DIST     = VERSION NOTES $(config_dist) $(pkg_dist) $(pkgdata_DATA)
DISTCLEANFILES = stamp-h
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
          configure libtool stamp-h.in config/ag_macros.m4

distcleancheck_listfiles = \
     find -type f -exec 'test -f $(srcdir)/{} || echo {} ;'

configure   : VERSION

pkg         : package
package     :
	cd pkg && $(MAKE) $@ pkgtype="$(pkgtype)"

usage-txt.po: gettext
gettext     :
	cd autoopts && $(MAKE) usage-txt.po

if HAVE_XML_LIB
release     : distcheck
else
release     :
	echo "Distributions cannot be made with a partial build" >&2
	exit 1
endif

## Makefile.am ends here
