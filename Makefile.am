##  -*- Mode: Makefile -*-
## Makefile.am --- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2009-07-22 19:00:50 bkorb"
##              by: bkorb
##
## This file is part of AutoGen.
## AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.9 2009/07/23 02:05:55 bkorb Exp $
## ---------------------------------------------------------------------
## Code:
ACLOCAL_AMFLAGS = -I config

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

.NOTPARALLEL:

## Makefile.am ends here
