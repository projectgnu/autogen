##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-10-28 16:14:22 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.4 2005/10/28 23:14:33 bkorb Exp $
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
aclocaldir     = $(datadir)/aclocal
aclocal_DATA   = config/liboptschk.m4
pkg_dist       = config/bootstrap.local
dist_data      = $(pkgdata_DATA) $(aclocal_DATA)
EXTRA_DIST     = VERSION NOTES $(config_dist) $(pkg_dist) $(dist_data)
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
