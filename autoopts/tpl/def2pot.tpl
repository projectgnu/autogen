[= AutoGen5 template pot                                     =][=
#
# this template can be used to generate .pot file for the
# option definition files for these templates:
#       aginfo.tpl, agman-cmd.tpl, agmdoc-cmd.tpl
#

##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2013 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following md5sums:
##
##  43b91e8ca915626ed3818ffb1b71248b COPYING.gplv3
##  06a1a2e4760c90ea5e1dad8dfaac4d39 COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f COPYING.mbsd

====================== FUNCTIONS BEGIN =======================][=
DEFINE genmsg                                                =][=

  IF (set! msg-id (get "msgid"))
     (set! msg-text (get-text msg-id))
     (< 0 (string-length msg-text))                          =]
#: [=(def-file-line msg-id "%s:%d")                          =]
msgid [= (c-string msg-text)                                 =]
msgstr ""
[=ENDIF                                                      =][=
ENDDEF                                                       =][=

DEFINE genmsg2                                               =][=
  IF (set! msg-text (get "msgid"))
     (string-length msg-text)                                =]
#: [=(def-file-line msg-text "%s:%d")                        =]
msgid [= (c-string msg-text)                                 =]
msgstr ""
[=ENDIF                                                      =][=
ENDDEF                                                       =][=

(define get-text (lambda (nm) (shell (string-append

 "{ sed 's/@[a-z]*{\\([^}]*\\)}/\\1/g' | "
    "${CLexe} --fill -I0 -W72\n"
 "} <<\\_EODesc_\n"
    (get nm)
    "\n_EODesc_"
))))
(define msg-text "")
(define msg-id   "")

  ;;# START-BUILDTREE-ISMS
  ;;
  (shell "CLexe=${AGexe%/agen5/*}/columns/columns
  test -x \"${CLexe}\" || {
    CLexe=${AGexe%/autogen}/columns
    test -x \"${CLexe}\" || die 'columns program is not findable'
  }")

=][= # END-BUILDTREE-ISMS

  (shell "CLexe=${AGexe%/autogen}/columns")

# END-INSTALL-ONLY-CODE =][= #

;; ==================== FUNCTIONS END ===========================

;; pot file header and comment info                          \=]
# localization template (.pot) for [= (def-file) =] of [= prog-name =],
# this file is used to generate localized manual for [= prog-name =].
# Copyright (C) [= (shell "date +%Y")                        =][=

  IF (exist? "copyright") =]
# This file is distributed under the terms of the
# [= (license-name (get "copyright.type")) \=]

# The program owners may be reached via:
#    [=(shellf
  "author='%s' email='%s' date=`date +%%Y`
  printf '%%s <%%s>, %%s.' \"$author\" \"$email\" \"${date}\""
    (get "copyright.owner" "FIRST AUTHOR")
    (get "copyright.eaddr" "EMAIL@ADDRESS")
)=][= ENDIF =]
#
#, fuzzy
msgid   ""
msgstr  ""
"Project-Id-Version: [= prog-name =] [= version =]\n"
"Report-Msgid-Bugs-To: \n"
"POT-Creation-Date: [= (shell "date +\"%F %R%z\"") =]\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
"Language-Team: LANGUAGE <LL@li.org>\n"
"Language: \n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=CHARSET\n"
"Content-Transfer-Encoding: 8bit\n"
[=
genmsg msgid=prog-title                                      =][=

FOR flag                                                     =][=
  genmsg msgid=descrip                                       =][=
  genmsg msgid=doc                                           =][=
ENDFOR                                                       =][=

FOR explain                                                  =][=
  genmsg msgid=explain                                       =][=
ENDFOR                                                       =][=

FOR doc-section                                              =][=
  genmsg msgid=ds-text                                       =][=
ENDFOR                                                       =][=

FOR prog-man-descrip                                         =][=
  genmsg msgid=prog-man-descrip                              =][=
ENDFOR                                                       =][=

FOR prog-info-descrip                                        =][=
  genmsg msgid=prog-info-descrip                             =][=
ENDFOR                                                       =][=

FOR detail                                                   =][=
  genmsg msgid=detail                                        =][=
ENDFOR                                                       =][=

FOR exit-desc                                                =][=
  genmsg msgid=exit-desc                                     =][=
ENDFOR                                                       =][=

CASE (get "copyright.type")                                  =][=
  = note                                                     =][=
  == ''                                                      =][=
  *                                                          =][=
    genmsg2 msgid=(string-append
        "This program is released under the terms of "
        (license-name (get "copyright.type")) ".")
                                                             =][=
ESAC                                                         =][=

genmsg msgid=option-info                                     =][=
genmsg msgid=argument                                        =][=
genmsg msgid=man-doc                                         =][=
genmsg msgid=copyright.text                                  =]
