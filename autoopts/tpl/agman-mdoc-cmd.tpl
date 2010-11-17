[+:   AutoGen5 template man=%s.1a

:+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : include
	#Start : description
	# * * * * * * * * * * * * * * * * * * * * * * * * *
				                  
				    :+]
.SH DESCRIPTION
[+:IF   (exist? "prog-man-descrip") :+][+:
  FOR prog-man-descrip  "\n\n"      :+][+:
    prog-man-descrip                :+][+:
  ENDFOR                            :+][+:
ELIF (exist? "prog-mdoc-descrip")   :+][+:
 (out-push-new detail-file)         :+][+:
  FOR prog-mdoc-descrip "\n\n"      :+][+:
    prog-mdoc-descrip               :+][+:
  ENDFOR                            :+][+:
ENDIF				    :+]
[+: IF (exist? "prog-mdoc-descrip") :+]
[+:
  (out-pop)
  :+] [+: (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+: 
ENDIF :+]
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
    prog-mdoc-bug 	            :+][+:
  ENDFOR			    :+][+:
ENDIF				    :+]
[+: IF (exist? "prog-mdoc-bug"):+]
[+:
  (out-pop)
  :+][+: (shellf "perl %s < %s" (find-file "mdoc2man.pl") detail-file) :+][+: 
ENDIF :+]
