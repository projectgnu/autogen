##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-07-24 12:45:15 bkorb"
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
##              by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.20 2004/08/12 03:29:15 bkorb Exp $
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
DISTCLEANFILES = .conf-stamp stamp-h
MAINTAINERCLEANFILES = COPYING INSTALL Makefile.in aclocal.m4 config-h.in \
          configure libtool stamp-h.in config/ag_macros.m4

distcleancheck_listfiles = \
     find -type f -exec 'test -f $(srcdir)/{} || echo {} ;'

configure   : VERSION

all         : .conf-stamp

.conf-stamp : configure
	BOOTSTRAP=addlink sh ${top_srcdir}/config/bootstrap.local
	@touch $@

pkg         : package
package     :
	cd pkg && $(MAKE) $@ pkgtype="$(pkgtype)"

release     :
if !HAVE_XML_LIB
	echo "Distributions cannot be made with a partial build" >&2
	exit 1
endif
	$(MAKE) distcheck
## Makefile.am ends here
