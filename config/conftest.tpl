[= AutoGen5 template -*- Mode: M4 -*-

null

#  Maintainer:        Bruce Korb <bkorb@gnu.org>
#  Created:           Tue Nov 24 01:07:30 1998
#  Last Modified:     $Date: 2002/02/03 01:29:28 $
#             by: bkorb
#
# This template uses the following definitions:
#
# 1.  group  - defines a prefix for the names.  The default is "ac".
# 2.  test   - an autoconf test to perform:
# 2.a  name  - name of the test
# 2.b  type  - "run", "link" or "compile"
# 2.c  check - short display name for user entertainment
# 2.d  code  - the test code to compile, link and/or run.
# 2.e  doc   - useful explanitory text
# 2.f  require - if there are conftest prerequisites
# 2.g  author  - [optional] name of test's author

(setenv "SHELL" "/bin/sh")

=][=

INCLUDE "confmacs.tpl"  =][=

(define ofile      "")
(define ofile-list "")  =][=
FOR test                =][=
    preamble            =][=
    (set! ofile (string-append (string-downcase mac-name) ".m4" ))
    (out-push-new ofile)
    (set! ofile-list (string-append ofile-list
          "\n" ofile))  =][=
    emit-macro          =][=
    (out-pop)           =][=
ENDFOR test             =][=

;; # # # # # # # # # Makefile.am # # # # # # # # # # #

(out-push-new "Makefile.am")
(dne "##  " "##  ")     =]
##
## ---------------------------------------------------------------------
## $Id: conftest.tpl,v 3.4 2002/02/03 01:29:28 bkorb Exp $
## ---------------------------------------------------------------------

GENERATED_M4 = \
[=(shellf "columns -I'\t' --spread=2 --line-sep=' \\' <<_EOF_\n%s\n_EOF_\n"
          ofile-list)=]

pkgdata_DATA = conftest.tpl confmacs.tpl

EXTRA_DIST = byacc.m4 libregex.m4 openmode.m4 missing bootstrap autogen.spec \
	$(pkgdata_DATA) $(GENERATED_M4) misc.def bootstrap.local

MAINTAINERCLEANFILES = Makefile.in config.guess config.sub install-sh \
	ltconfig ltmain.sh missing mkinstalldirs $(GENERATED_M4)
all:
	:
[=

(out-pop)

;; end conftest.tpl     =]
