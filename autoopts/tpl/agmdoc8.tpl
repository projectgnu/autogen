[+: -*- Mode: nroff -*- 
 AutoGen5 template man=%s.8  			  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#Start  : definitions
	# * * * * * * * * * * * * * * * * * * * * * * * * *
				
				                  :+][+:
 
 (define man-sect "8") 
 (define prog-name (string-downcase! (get "prog-name")))
 (define PROG_NAME (string-tr! (get "prog-name") "a-z^-" "A-Z__"))
 (define prog_name (string-tr! (get "prog-name") "A-Z^-" "a-z__"))  
 (make-tmp-dir)
 (define detail-file (string-append tmp-dir "/detail"))

						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : Definition
	#Start : Environment Variable Setting
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+][+:

 (setenv "SHELL" "/bin/sh") 
						  :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : Environment Variable Setting
	#Start : Date
	# * * * * * * * * * * * * * * * * * * * * * * * * *

				                  :+].Dd [+:

 `date +%Y-%m-%d`

						  :+].Os [+:

 %os (string-upcase! "%s") 

:+] [version/release]
.Dt [+:

 %prog-name (string-upcase! "%s") 

:+] [+:

 (. man-sect) 

:+]
.\"  
.\"  new section
.\"
.Sh NAME
.Nm [+:

 prog-name 

:+]
.Nd [+:

 prog-title 

:+]
[+:

;; The following "dne" argument is a string of 5 characters:
;; '.' '\\' '"' and two spaces.  It _is_ hard to read.
;;
(dne ".\\\"  ") :+][+:  # balance quotes for emacs: "

:+]
.\"  
.\"  new section
.\"
.Sh SYNOPSIS
.Nm [+:

 %prog-name (string-upcase! "%s") 

:+][+:

    (define test (if (exist? "prog-name") #t #f)) :+][+:
    (define named-mode (not (or test (exist? "long_opts" )))) :+][+:
    IF (and test (exist?  "flag.value") ) :+][+:
	flag.value 

:+][+:

    ENDIF

:+]
.\"
.\" new section
.\"
.Sh DESCRIPTION
[+:
IF   (exist? "prog-mdoc-descrip ")  :+][+:
  FOR prog-mdoc-descrip  "\n\n"     :+][+:
    prog-mdoc-descrip               :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-man-descrip ")   :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-man-descrip "\n\n"       :+][+:
    prog-man-descrip                :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-info-descrip ")  :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-descrip  "\n\n"     :+][+:
    prog-info-descrip               :+][+:
  ENDFOR                            :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-man-descrip ")   :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "man2mdoc.pl") detail-file) :+][+:
ELIF (exist? "prog-info-descrip")   :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "texi2mdoc.pl") detail-file) :+][+:
ENDIF
:+]
.Sh FILES
[+:
IF   (exist? "prog-mdoc-files ")    :+][+:
  FOR prog-mdoc-files  "\n\n"       :+][+:
    prog-mdoc-files                 :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-man-files ")     :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-man-files "\n\n"         :+][+:
    prog-man-files                  :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-info-files ")    :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-files  "\n\n"       :+][+:
    prog-info-files                 :+][+:
  ENDFOR                            :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-man-files ")     :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "man2mdoc.pl") detail-file) :+][+:
ELIF (exist? "prog-info-files")     :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "texi2mdoc.pl") detail-file) :+][+:
ENDIF
:+]
.Sh SEE ALSO
[+:
IF   (exist? "prog-mdoc-see-also ") :+][+:
  FOR prog-mdoc-see-also  "\n\n"    :+][+:
    prog-mdoc-see-also              :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-man-see-also ")  :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-man-see-also "\n\n"      :+][+:
    prog-man-see-also 	            :+][+:
  ENDFOR			    :+][+:
ELIF (exist? "prog-info-see-also ") :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-see-also  "\n\n"    :+][+:
    prog-info-see-also              :+][+:
  ENDFOR			    :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-man-see-also ")  :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "man2mdoc.pl") detail-file) :+][+:
ELIF (exist? "prog-info-see-also")  :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "texi2mdoc.pl") detail-file) :+][+:
ENDIF
:+]
.Sh BUGS
[+:
IF   (exist? "prog-mdoc-bug")       :+][+:
  FOR prog-mdoc-bug  "\n\n"         :+][+:
    prog-mdoc-bug                   :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-man-bug")        :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-man-bug "\n\n"           :+][+:
    prog-man-bug                    :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-info-bug")       :+][+:
  (out-push-new detail-file)        :+][+:
  FOR prog-info-bug  "\n\n"         :+][+:
    prog-info-bug                   :+][+:
  ENDFOR                            :+][+:
ENDIF
                                    :+][+:
IF   (exist? "prog-man-bug")        :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "man2mdoc.pl") detail-file) :+][+:
ELIF (exist? "prog-info-bug")       :+][+:
  (out-pop)                         :+][+:
  (shellf "perl %s < %s" (find-file "texi2mdoc.pl") detail-file) :+][+:
ENDIF
:+]