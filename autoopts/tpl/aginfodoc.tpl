[= AutoGen5 template texi -*- Mode: texi -*-

(setenv "SHELL" "/bin/sh")

=]
\input texinfo   @c -*-texinfo-*-
@c %**start of header[=

  # * * * * * * * * * * * * * * * * * * * * * * * * *
  #Start: definitions
  # * * * * * * * * * * * * * * * * * * * * * * * * *

=][=

  (define down-prog-name  (string-downcase (get "prog-name")))
  (define file-name       (string-append down-prog-name ".info"))
  (define coded-prog-name (string-append "@code{" down-prog-name "}"))

  (define replace-prog-name (lambda (nm)
        (string-substitute (get nm)
              down-prog-name coded-prog-name )  ))

  =][=# * * * * * * * * * * * * * * * * * * * * * * *
  #end  : definitions
  # * * * * * * * * * * * * * * * * * * * * * * * * *

  # * * * * * * * * * * * * * * * * * * * * * * * * *
  #start: filename section
  # * * * * * * * * * * * * * * * * * * * * * * * * *

=]
@setfilename [= (. file-name) =][=

  # * * * * * * * * * * * * * * * * * * * * * * * * *
  #end  : filename section
  # * * * * * * * * * * * * * * * * * * * * * * * * *

  # * * * * * * * * * * * * * * * * * * * * * * * * *
  #start: program titile section
  # * * * * * * * * * * * * * * * * * * * * * * * * *

=]
@settitle [= prog-title =][=

  # * * * * * * * * * * * * * * * * * * * * * * * * *
  #end : program titile section
  # * * * * * * * * * * * * * * * * * * * * * * * * *

=]
@c %**end of header[=

  IF (exist? "texi-set-chapter-new-page")
=]
@setchapternewpage [= texi-set-chapter-new-page =][=
  ENDIF
=]
@titlepage
@sp 10
@comment The title is printed in a large font.
@center @titlefont{Sample Title}

@c The following two commands start the copyright page.
@page
@vskip 0pt plus 1filll
@ignore
[=

(dne "# " "# ")

=]
@end ignore
@copyright{} 1990 Free Software Foundation, Inc.
[= IF (exist? "copyright") =]

This software is released under [=

  CASE copyright.type
=][=

   =  gpl
=]the GNU General Public License[=

   = lgpl
=]the GNU General Public License with Library Extensions[=

   =  bsd

=]the Free BSD License[=

   *
=]a specialized copyright license[=

  ESAC =].[=

ENDIF

=]
@end titlepage

@node [= prog-name =] Invocation
@[=

  (define doc-level (getenv "LEVEL"))

  (if (not (defined? 'doc-level))
          (set! doc-level "unnumberedsec")

      (if (not (string? doc-level))
          (set! doc-level "unnumberedsec"))

  doc-level =] Invoking [= prog_name =]

@pindex [= prog-name  =]
@cindex [= prog-title =][=

FOR concept

=]
@cindex [= concept =][=

ENDFOR

=]
[=
        IF (exist? "prog-info-descrip")  =][=
                FOR prog-info-descrip  "\n\n"  =][=
                prog-info-descrip            =][=
        ENDFOR                         =][=
                ELIF (exist? "detail")           =][=
        (replace-prog-name "detail")   =][=
        ENDIF
=]
@menu
* [=

        (sprintf "%s %-24s %s" down-prog-name "detais::" (get "prog-name"))

=]detais of ntpq command
* [=

        (sprintf "%s %-24s %s" down-prog-name "usage::" (get "prog-name"))

=]usage help[=

     (if (exist? "flag.value") " (-?)")

=]
[=

        (out-push-new)
=][=

FOR flag

=][=

   IF (not (exist? "documentation"))

=]* [=
     (sprintf "%s %-24s" down-prog-name
           (string-append(string-tr! (get "name") "A-Z^_" "a-z--" ) "::" ) )

=][=

    % name (string-tr! "%s" "A-Z^_" "a-z--")=] option[=

    % value " (-%s)"

=]
[=

   ENDIF       =][=

ENDFOR flag    =][=

(shell (string-append "sort <<-\\_EOF_\n" (out-pop #t) "\n_EOF_"))

=]
@end menu
[= ?% explain %s "This program has no explanation.\n" =]


@node [= (sprintf "%s %-24s" down-prog-name "detais") =]
@[= CASE (. doc-level) =][=
        = unnumbered    =][=
        = unnumberedsec =]unnumberedsubsec[=
        = unnumberedsubsec =]unnumberedsubsubsec[=
        ESAC =] [= down-prog-name =] Details
@cindex [= prog-name =] Details


@node [= (sprintf "%s %-24s" down-prog-name "usage") =]
@[= CASE (. doc-level) =][=
        = unnumbered    =][=
        = unnumberedsec =]unnumberedsubsec[=
        = unnumberedsubsec =]unnumberedsubsubsec[=
        ESAC =] [= prog-name =] Usage
@cindex [= (. down-prog-name) =] Usage



[=
IF (define opt-name "")
   (define extra-ct 0)
   (define extra-text "")
   (exist? "preserve-case")     =][=
   (define optname-from "_^")
   (define optname-to   "--")   =][=
ELSE                            =][=
  (define optname-from "A-Z")
  (define optname-to   "a-z")   =][=
ENDIF                           =][=

FOR flag                        =][=

  IF (not (exist? "documentation")) =]

@node [=
    (set! opt-name (string-tr (get "name") optname-from optname-to))
    (string-append down-prog-name " " opt-name)=]
@[=CASE (. doc-level) =][=
   = unnumbered       =][=
   = unnumberedsec    =]unnumberedsubsec [=
   = unnumberedsubsec =]unnumberedsubsubsec [=
   ESAC =][=(. opt-name)=] option[=
          % value " (-%s)" =]
@cindex [=(. down-prog-name)=]-[= (. opt-name) =]
This is the ``[=(string-downcase! (get "descrip"))=]'' option.
[= ?% doc "\n%s" "\nThis option has no @samp{doc} documentation." =]
[=ENDIF=][=

ENDFOR flag

=]
[= (. opt-name) =]
[= (set! extra-ct (+ extra-ct 1)) =]
[= (. extra-ct) =]
[= flag.test =]
[= FOR flag (for-from 1) (for-to 3) =]
[= flag.name =]
[= ENDFOR=]
[= prog-detail =]
