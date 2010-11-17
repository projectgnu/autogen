[+: -*- Mode: nroff -*-
  AutoGen5 template man=%s.8			  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#Start  : definitions
	# * * * * * * * * * * * * * * * * * * * * * * * * *
				
				                  :+][+:
 (define man-sect "5") 
 (define down_prog-name (string-downcase! (get "prog-name")))
 (define UP_PROG_NAME (string-upcase! (get "prog-name")))
 (make-tmp-dir)
 (define detail-file (string-append tmp-dir "/detail"))
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : Definition
	#Start : Environment Variable Setting
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+][+:
 #(setenv "SHELL" "/bin/sh")
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : Environment Variable Setting
	#Start : program name
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+][+:
 `printf '.TH' `				  
						  :+] [+: 
 (. UP_PROG_NAME)
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : program name
	#Start : man section
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+] [+:
 (. man-sect) 
						  :+][+:
 
	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : man section
	#Start : date
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+] [+:
 `date +%Y-%m-%d` 
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : date
	#Start : package and version
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+][+:
 IF (or (exist? "package") (exist? "version"))
						  :+][+:
 	`printf ' "(' `				  
						  :+][+:
 	package 
						  :+] [+:
 	version 
						  :+][+:
        `printf ')" ' `				  
						  :+][+:
 ENDIF 
					          :+][+:
 `printf ' "Programmer Manual"' `				  
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : package and version
	#Start : do not edit argument
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+]
[+:
 (dne ".\\\"  ") 				  
						  :+]
.\"
[+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : do not edit argument
	#Start : program name and title
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  
						  :+][+:
 	`printf '.SH NAME\n.B' `				  
						  :+] [+:
 prog-name 
:+] \- 
[+:
 prog-title 
:+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : program name and title
	#Start : synopsis
	# * * * * * * * * * * * * * * * * * * * * * * * * *
				                  
						  :+]
[+:
 `printf '.SH SYNOPSIS\n.B' `				  
						  	:+] [+:
 prog-name
							:+][+:
 (define use-flags (if (exist? "flag.value") #t #f))						
							:+][+:
 IF (define named-mode (not (or use-flags (exist? "flag.long_opts") )))
 use-flags  
							:+][+:
  	IF (exist? "flag.long_opts")			
						        :+]
.\" Mixture of short (flag) options and long options
.RB [ -\fIflag\fP " [\fIvalue\fP]]... [" --\fIopt-name\fP " [[=| ]\fIvalue\fP]]..."[+:

    	ELSE no long options:				     
							 :+]
.\" Short (flag) options only
.RB [ -\fIflag\fP " [\fIvalue\fP]]..."[+:
        ENDIF 						
							:+][+:
  ELIF (exist? "long_opts") 				
							:+]
.\" Long options only
.RB [ --\fIopt-name\fP [ = "| ] \fIvalue\fP]]..."[+:

  ELIF  (not (exist? "argument")) 			
							:+]
.\" All arguments are named options.
.RI [ opt-name "[\fB=\fP" value ]]...
.PP
.\"All arguments are named options.			   [+:

  ELSE 
							:+][+:

    (error "Named option programs cannot have arguments")

 							:+][+:
  ENDIF 
							:+][+:

  IF (exist? "argument") 

:+]
.br
.in +8
[+:

 argument 

:+][+:

    IF (exist? "reorder-args") 

:+]
.br
Operands and options may be intermixed.  They will be reordered.
[+: 

ENDIF 

:+][+:

  ELIF (or (exist? "long_opts") use-flags) 

:+]
.PP
All arguments must be options.[+:

  ENDIF 

:+][+:

      # * * * * * * * * * * * * * * * * * * * * * * * * *
      #
      #  Describe the command.  Use 'prog_man_desrip' if it exists,
      #  otherwise use the 'detail' help option.  If not that,
      #  then the thing is undocumented.
      #
      

:+][+:

IF (exist? "explain") 

:+]
.PP
[+:

explain

:+][+:

ENDIF 

:+]
.SH DESCRIPTION
[+:IF   (exist? "prog-man-descrip")     :+][+:
  FOR prog-man-descrip  "\n\n"  :+][+:
    prog-man-descrip            :+][+:
  ENDFOR                        :+][+:
ELIF (exist? "prog-mdoc-descrip"):+][+:
 (out-push-new detail-file)    :+][+:
  FOR prog-mdoc-descrip "\n\n"   :+][+:
    prog-mdoc-descrip 	        :+][+:
  ENDFOR
:+][+:
ENDIF
:+]
[+: IF (exist? "prog-mdoc-descrip"):+]
[+:
  (out-pop)
  :+] [+: (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+: 
ENDIF :+][+:

IF (exist? "main")   :+][+:

  IF (= (get "main.main-type") "for-each")  :+]
[+:
    CASE main.handler-type  :+][+:
    ~* ^(name|file)|.*text  :+]
This program will perform its function for every file named on the command
line or every file named in a list read from stdin.  The arguments or input
names must be pre-existing files.  The input list may contain comments,
which[+:

    !E        :+]
This program will perform its function for every command line argument
or every non-comment line in a list read from stdin.  The input list comments[+:

    ESAC      :+]
are blank lines or lines beginning with a '[+:
 ?% comment-char "%s" "#" :+]' character.[+:

  ENDIF       :+][+:

ENDIF - "main" exists

:+]
.SH OPTIONS[+:

;; * * * * * * * * * * * * * * * * * * * * * * * * *
;;
;; Describe each option
;;
(define opt-arg  "")
(define dis-name "")
(define opt-name "")
(if (exist? "preserve-case")
    (begin
      (define optname-from "_^")
      (define optname-to   "--") )
    (begin
      (define optname-from "A-Z_^")
      (define optname-to   "a-z--") )
)
(if (exist? "option-info")
    (string-append "\n.PP\n" (get "option-info") "\n") ) :+][+:

FOR flag

:+][+:

  IF

  (if (exist? "enable")
      (set! opt-name (string-append (get "enable") "-" (get "name")))
      (set! opt-name (get "name")) )
  (if (exist? "disable")
      (set! dis-name (string-append (get "disable") "-" (get "name")))
      (set! dis-name "") )

  (set! opt-name (string-tr! opt-name optname-from optname-to))
  (set! dis-name (string-tr! dis-name optname-from optname-to))

  (if (not (exist? "arg-type"))
      (set! opt-arg "")
      (set! opt-arg (string-append "\\fI"
            (if (exist? "arg-name") (get "arg-name")
                (string-downcase! (get "arg-type")))
            "\\fP" ))
  )

  (exist? "documentation")

:+]
.SS "[+: descrip :+]"[+:

  ELSE *NOT* documentation

:+]
.TP[+:
    IF (exist? "value") :+][+:
      IF (exist? "long-opts") :+][+:

          # * * * * * * * * * * * * * * * * * * * *
          *
          *  The option has a flag value (character) AND
          *  the program uses long options
          *
          :+]
.BR -[+:value:+][+:
          IF (not (exist? "arg-type")) :+] ", " --[+:
          ELSE  :+] " [+:(. opt-arg):+], " --[+:
          ENDIF :+][+: (. opt-name)    :+][+:
          IF (exist? "arg-type")       :+][+:
              ? arg-optional " [ =" ' "=" '
              :+][+:  (. opt-arg)      :+][+:
              arg-optional " ]"        :+][+:
          ENDIF :+][+:
          IF (exist? "disable") :+], " \fB--[+:(. dis-name):+]\fP"[+:
          ENDIF :+][+:

        ELSE   :+][+:

          # * * * * * * * * * * * * * * * * * * * *
          *
          *  The option has a flag value (character) BUT
          *  the program does _NOT_ use long options
          *
          :+]
.BR -[+:value:+][+:
          IF (exist? "arg-type") :+][+:
            arg-optional "["     :+] "[+:(. opt-arg):+][+:
            arg-optional '"]"'   :+][+:
          ENDIF " :+][+:
        ENDIF     :+][+:


      ELSE  value does not exist -- named option only  :+][+:

        IF (not (exist? "long-opts")) :+][+:

          # * * * * * * * * * * * * * * * * * * * *
          *
          *  The option does not have a flag value (character).
          *  The program does _NOT_ use long options either.
          *  Special magic:  All arguments are named options.
          *
          :+]
.BR [+: (. opt-name) :+][+:
          IF (exist? "arg-type") :+] [+:
             ? arg-optional " [ =" ' "=" '
             :+][+:(. opt-arg):+][+:
             arg-optional "]" :+][+:
          ENDIF:+][+:
          IF (exist? "disable") :+], " \fB[+:(. dis-name):+]\fP"[+:
          ENDIF :+][+:


        ELSE   :+][+:

          # * * * * * * * * * * * * * * * * * * * *
          *
          *  The option does not have a flag value (character).
          *  The program, instead, only accepts long options.
          *
          :+]
.BR --[+: (. opt-name) :+][+:
          IF (exist? "arg-type") :+] "[+: #" :+][+:
            arg-optional "["     :+]=[+:(. opt-arg):+][+:
            arg-optional "]"     :+]"[+: #" :+][+:
          ENDIF :+][+:
          IF (exist? "disable") :+], " \fB--[+:(. dis-name):+]\fP"[+:
          ENDIF :+][+:
        ENDIF   :+][+:
      ENDIF     :+]
[+: (string-substitute (get "descrip") "\\" "\\\\") :+].[+:
      IF (exist? "min") :+]
This option is required to appear.[+:ENDIF:+][+:
      IF (exist? "max") :+]
This option may appear [+:
          IF % max (= "%s" "NOLIMIT")
          :+]an unlimited number of times[+:ELSE
          :+]up to [+:max:+] times[+:
          ENDIF:+].[+:
      ENDIF:+][+:
      IF (exist? "disable") :+]
The \fI[+:(. dis-name):+]\fP form will [+:
         IF (exist? "stack-arg") :+]clear the list of option arguments[+:
         ELSE  :+]disable the option[+:
         ENDIF :+].[+:
      ENDIF:+][+:
      IF (exist? "enabled") :+]
This option is enabled by default.[+:ENDIF:+][+:
      IF (exist? "no-preset") :+]
This option may not be preset with environment variables
or in initialization (rc) files.[+:ENDIF:+][+:
      IF (and (exist? "default") named-mode) :+]
This option is the default option.[+:
      ENDIF:+][+:
      IF (exist? "equivalence") :+]
This option is a member of the [+:equivalence:+] class of options.[+:ENDIF:+][+:
      IF (exist? "flags-must") :+]
This option must appear in combination with the following options:
[+: FOR flags-must ", " :+][+:flags-must:+][+:ENDFOR:+].[+:ENDIF:+][+:
      IF (exist? "flags-cant") :+]
This option must not appear in combination with any of the following options:
[+: FOR flags-cant ", " :+][+:flags-cant:+][+:ENDFOR:+].[+:
      ENDIF     :+][+:


      IF (~* (get "arg-type") "key|set") :+]
This option takes a keyword as its argument[+:

         IF (=* (get "arg-type") "set")

:+] list.  Each entry turns on or off
membership bits.  The bits are set by name or numeric value and cleared
by preceding the name or number with an exclamation character ('!').
They can all be cleared with the magic name \fInone\fR and they can all be set
with
.IR all .
A single option will process a list of these values.[+:

         ELSE

:+].  The argument sets an enumeration value that can
be tested by comparing them against the option value macro.[+:

         ENDIF

:+]
The available keywords are:
.in +4
.nf
.na
[+: (shellf "${CLexe:-columns} --spread=1 -W50 <<_EOF_\n%s\n_EOF_"
            (join "\n" (stack "keyword"))  )   :+]
.fi
or their numeric equivalent.
.in -4[+: (if (exist? "arg-default") "\n.sp" ) :+][+:

      ELIF (=* (get "arg-type") "num")         :+]
This option takes an integer number as its argument.[+:

        IF  (exist? "arg-range")  :+]
The value of [+:(. opt-arg):+] is constrained to being:
.in +4
.nf
.na[+:     FOR arg_range ", or"   :+]
[+: (shellf "
range='%s'

case \"X${range}\" in
X'->'?*  )
  echo \"less than or equal to\" `
    echo $range | sed 's/->//' ` ;;

X?*'->'  )
  echo \"greater than or equal to\" `
    echo $range | sed 's/->.*//' ` ;;

X?*'->'?* )
  echo \"in the range \" `
    echo $range | sed 's/->/ through /' ` ;;

X?* )
  echo exactly $range ;;

X* ) echo $range is indeterminate
esac"

(get "arg-range") )
:+][+:
           ENDFOR arg-range :+]
.fi
.in -4[+:

        ENDIF   :+][+:

      ENDIF     :+][+:

      IF (exist? "arg-default") :+]
The default [+: (. opt-arg) :+] for this option is:
.ti +4
 [+: (join " + " (stack "arg-default" )) :+][+:
      ENDIF     :+]
.sp
[+:
 (if (exist? "doc")
        (string-substitute (get "doc") "\\" "\\\\")
        "This option has not been fully documented." ) :+][+:

  ENDIF (not (exist? "documentation")) :+][+:

ENDFOR flag


:+]
.TP
.BR [+:

  IF (. use-flags)  :+]\-[+: ?% help-value "%s" "?" :+][+:
     IF (exist? "long-opts") :+] , " \--help"[+: ENDIF :+][+:
  ELSE   :+][+:
     IF (exist? "long-opts") :+]\--[+: ENDIF :+]help[+:
  ENDIF  :+]
Display usage information and exit.
.TP
.BR [+:

  IF (. use-flags)  :+]\-[+: ?% more-help-value "%s" "!" :+][+:
     IF (exist? "long-opts") :+] , " \--more-help"[+: ENDIF :+][+:
  ELSE   :+][+:
     IF (exist? "long-opts") :+]\--[+: ENDIF :+]more-help[+:
  ENDIF  :+]
Extended usage information passed thru pager.[+:


IF (exist? "homerc") :+]
.TP
.BR [+:

  IF (. use-flags)  :+]\-[+: ?% save-opts-value "%s" ">"
     :+] " [\fIrcfile\fP][+:
     IF (exist? "long-opts") :+]," " \--save-opts" "[=\fIrcfile\fP][+:
     ENDIF :+]"[+:
  ELSE     :+][+:
     IF (exist? "long-opts") :+]\--[+:
     ENDIF :+]save-opts "[=\fIrcfile\fP]"[+:
  ENDIF    :+]
Save the option state to \fIrcfile\fP.  The default is the \fIlast\fP
configuration file listed in the \fBOPTION PRESETS\fP section, below.
.TP
.BR [+:

  IF (. use-flags)  :+]\-[+: ?% load-opts-value "%s" "<"
     :+] " \fIrcfile\fP[+:
     IF (exist? "long-opts")
           :+]," " \--load-opts" "=\fIrcfile\fP," " --no-load-opts[+:
     ENDIF :+]"[+:
  ELSE     :+][+:
     IF (exist? "long-opts") :+]\--[+:
     ENDIF :+]load-opts "=\fIrcfile\fP," " --no-load-opts"[+:
  ENDIF    :+]
Load options from \fIrcfile\fP.
The \fIno-load-opts\fP form will disable the loading
of earlier RC/INI files.  \fI--no-load-opts\fP is handled early,
out of order.[+:
ENDIF (exist? "homerc") :+][+:


IF (exist? "version") :+]
.TP
.BR [+:

  IF (. use-flags)  :+]\-[+: ?% version-value "%s" "v"
     :+] " [{\fIv|c|n\fP}][+:
     IF (exist? "long-opts") :+]," " \--version" "[=\fI{v|c|n}\fP][+:
     ENDIF :+]"[+:
  ELSE     :+][+:
     IF (exist? "long-opts") :+]\--[+:
     ENDIF :+]version "[=\fI{v|c|n}\fP]"[+:
  ENDIF    :+]
Output version of program and exit.  The default mode is `v', a simple
version.  The `c' mode will print copyright information and `n' will
print the full copyright notice.[+:
ENDIF


:+][+:
IF (or (exist? "homerc") (exist? "environrc")) :+]
.SH OPTION PRESETS
Any option that is not marked as \fInot presettable\fP may be preset
by loading values from [+:
  IF (exist? "homerc")
    :+]configuration ("RC" or ".INI") file(s)[+:
    IF (exist? "environrc")
            :+] and values from
[+:
    ENDIF   :+][+:
  ENDIF     :+][+:
  IF (exist? "environrc") :+]environment variables named:
.nf
  \fB[+: (. PROG_NAME) :+]_<option-name>\fP or \fB[+: (. PROG_NAME) :+]\fP
.fi
.ad[+:
    IF (exist? "homerc") :+]
The environmental presets take precedence (are processed later than)
the configuration files.[+:
    ENDIF   :+][+:
  ELSE      :+].[+:
  ENDIF     :+][+:

  CASE
    (define rc-file (if (exist? "rcfile") (get "rcfile")
                    (string-append "." prog_name "rc")  ))
    (count "homerc") :+][+:
  == "0"    :+][+:
  == "1"    :+]
The \fIhomerc\fP file is "\fI[+:homerc:+]\fP", unless that is a directory.
In that case, the file "\fI[+: (. rc-file) :+]\fP"
is searched for within that directory.[+:

  *         :+]
The \fIhomerc\fP files are [+:
    FOR homerc ", "  :+][+:
      IF (last-for?) :+]and [+:
      ENDIF :+]"\fI[+: homerc :+]\fP"[+: ENDFOR :+].
If any of these are directories, then the file \fI[+: (. rc-file) :+]\fP
is searched for within those directories.[+:
  ESAC      :+][+:


ENDIF       :+][+:
IF (exist? "man-doc") :+]
[+:man-doc:+][+:
ENDIF:+][+:

IF (define tmp-str (get "copyright.author" (get "copyright.owner")))
   (> (string-length tmp-str) 0) :+]
.SH AUTHOR
[+: (. tmp-str) :+][+:

 (set! tmp-str (get "copyright.eaddr" (get "eaddr")))
 (if (> (string-length tmp-str) 0)
   (string-append "\n.br\nPlease send bug reports to:  " tmp-str "\n") ) :+][+:

  CASE copyright.type :+][+:
   =  gpl  :+]
.PP
Released under the GNU General Public License.[+:
   = lgpl  :+]
.PP
Released under the GNU General Public License with Library Extensions.[+:
   =  bsd  :+]
.PP
Released under the Free BSD License.[+:
   *       :+][+:
     IF (exist? "copyright.text")
           :+]
.PP
.nf
.na
[+: copyright.text :+]
.fi
.ad[+:
     ELIF (exist? "copyright.date") :+]
.PP
Released under an unspecified copyright license.[+:
     ENDIF                          :+][+:
  ESAC                              :+][+:
ENDIF      :+]
.PP
This manual page was \fIAutoGen\fP-erated from the \fB[+: prog-name :+]\fP
option definitions.[+: #

agman1.tpl ends here  :+]
.SH FILES
[+:
IF   (exist? "prog-man-files ")     :+][+:
  FOR prog-man-files  "\n\n"        :+][+:
    prog-man-files                  :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-mdoc-files ")    :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-mdoc-files "\n\n"        :+][+:
    prog-mdoc-files                 :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-info-files ")    :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-files  "\n\n"       :+][+:
    prog-info-files                 :+][+:
  ENDFOR                            :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-mdoc-files ")    :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+:
ELIF (exist? "prog-info-files")     :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "texi2mdoc.pl") detail-file) :+][+:
ENDIF
:+]
.SH SEE ALSO
[+:
IF   (exist? "prog-man-see-also ")  :+][+:
  FOR prog-man-see-also  "\n\n"     :+][+:
    prog-man-see-also               :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-mdoc-see-also ") :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-mdoc-see-also "\n\n"     :+][+:
    prog-mdoc-see-also              :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-info-see-also ") :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-see-also  "\n\n"    :+][+:
    prog-info-see-also              :+][+:
  ENDFOR                            :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-mdoc-see-also ") :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+:
ELIF (exist? "prog-info-see-also")  :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+:
ENDIF
:+]
.SH BUGS
[+:IF   (exist? "prog-man-bug")     :+][+:
  FOR prog-man-bug  "\n\n"          :+][+:
    prog-man-bug                    :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-mdoc-bug")       :+][+:
 (out-push-new detail-file)         :+][+:
  FOR prog-mdoc-bug "\n\n"          :+][+:
    prog-mdoc-bug                   :+][+:
  ENDFOR
                                    :+][+:
ENDIF
:+]
[+: IF (exist? "prog-mdoc-bug"):+]
[+:
  (out-pop)
:+][+: (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+: 
ENDIF :+]
