[+: -*- Mode: nroff -*-
  AutoGen5 template man=%s.5			  :+][+:

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
         argument                                 :+][+:

	# * * * * * * * * * * * * * * * * * * * * * * * * *
	#End   : synopsis
	#Start : common fileds
	# * * * * * * * * * * * * * * * * * * * * * * * * *
				                  
						  :+]
[+: INCLUDE "agman-mdoc-cmd.tpl" :+]
