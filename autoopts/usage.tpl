[=  AutoGen5 Template  -*- Mode: shell-script -*-

 help-text

=][=

 ;; This template is designed to emit help text from the current set
 ;; of option definitions.
 ;;
 (make-tmp-dir)
 (out-push-new (shellf "echo ${tmp_dir}/%s.def" (get "prog-name")))

=]
AutoGen Definitions options.tpl;
[=

FOR var IN  prog-name prog-title argument      config-header
            environrc export     homerc        include
            long-opts rcfile     version       detail
            explain   package    preserve-case prog-desc
            opts-ptr  gnu-usage  reorder-args  usage-opt

             version-value help-value more-help-value usage-value
             save-opts-value load-opts-value
  =][=

  IF (exist? (get "var"))   =]
[= var =] = [= (kr-string (get (get "var"))) =];[=
  ENDIF have "var"          =][=

ENDFOR var IN ....          =][=

FOR copyright               =]
copyright = {[=

  FOR var IN  date owner type text author eaddr
    =][=

    IF (exist? (get "var")) =]
    [= var =] = [=
       (kr-string (get (string-append "copyright." (get "var"))))=];[=
    ENDIF have "var"        =][=

  ENDFOR var IN ....        =]
};[=
ENDFOR copyright            =]

main = {
  main-type = shell-process;
};

[=

FOR flag

=]
flag = {[=

  FOR var IN name descrip value max min must-set enable disable enabled
             ifdef ifndef no-preset settable equivalence documentation
             immediate immed-disable also
             arg-type arg-optional arg-default default arg-range
             stack-arg unstack-arg
    =][=

    IF (exist? (get "var")) =]
    [= var =] = [= (kr-string (get (get "var"))) =];[=
    ENDIF have "var"        =][=

  ENDFOR var IN ....        =][=

  IF (exist? "keyword")     =]
    keyword = [= (join "," (stack "keyword")) =];[=
  ENDIF  keyword exists     =][=

  IF (exist? "flags-must")  =]
    flags-must = [= (join "," (stack "flags-must")) =];[=
  ENDIF  flags-must exists  =][=

  IF (exist? "flags-cant")  =]
    flags-cant = [= (join "," (stack "flags-cant")) =];[=
  ENDIF  flags-cant exists  =]
};[=

ENDFOR flag

=][=

(out-pop)
(out-push-new)  \=]

cd ${tmp_dir}
defs=-DTEST_[= (string-upcase! (string->c-name! (get "prog-name"))) =]_OPTS=1
cflags=`autoopts-config cflags`
ldflags=`autoopts-config ldflags`
flags="${defs} ${cflags}"

autogen [= prog-name=].def || die "Cannot gen [= prog-name =]"
${CC:-cc} ${flags} -g -o [= prog-name =] [= prog-name =].c ${ldflags} || \
  die cannot compile [= prog-name =].c in `pwd`
./[= prog-name =] [=

CASE usage-type =][=
== short        =][=
   IF (exist? "usage-opt") =][=
      (if (exist? "long-opts") "--usage"
          (string-append "-"
              (if (exist? "usage-value") (get "usage-value") "u")) ) =][=
   ELSE no usage-opt 
      =]--give-me-short-usage 2>&1 | sed /give-me-short-usage/d[=
   ENDIF        =][=

*               =][=

  (if (exist? "long-opts") "--help"
      (string-append "-" (if (exist? "help-value") (get "help-value") "?")) )

=][=

ESAC      =] || \
  die cannot obtain [= prog-name =] help in `pwd`[=

(shell (out-pop #t))

=]
[=

# end of usage.tpl      \=]
