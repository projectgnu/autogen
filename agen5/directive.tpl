[= AutoGen5 template -*- Mode: C -*-
# $Id: directive.tpl,v 3.4 2003/05/17 02:45:19 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h =]
[=(dne " *  " "/*  ")=]
 *
 *  copyright 1992-2003 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 */
[=(make-header-guard "directive")=]
#ifdef DEFINING

typedef char* (tDirProc)( char* pzArg, char* pzScan );

typedef struct dir_table tDirTable;
struct dir_table {
    size_t      nameSize;
    tCC*        pzDirName;
    tDirProc*   pDirProc;
    int         unused;
};

/*
 *  Declare the procedures that will handle the directives
 */
static tDirProc doDir_IGNORE;[=
FOR directive    =][=
  IF (not (exist? "dummy")) =]
static tDirProc doDir_[=name=];[=
  ENDIF          =][=
ENDFOR directive =]

/*
 *  Define the constant string names for each directive
 */[=
FOR directive    =]
static const char z[=% name (string-capitalize! (sprintf "%%-12s" "%s[]"))
       =] = "[=% name (string-downcase! "%s") =]";[=
ENDFOR directive
=]

/*
 *  Enumerate the directives
 */
typedef enum {[=
FOR directive , =]
    DIR_[=% name (string-upcase! "%s") =][=
ENDFOR directive=]
} teDirectives;

#define DIRECTIVE_CT  [= (+ (high-lim "directive") 1) =]
static tDirTable dirTable[ DIRECTIVE_CT ] = {[=
FOR directive , =]
    { sizeof( z[=% name (string-capitalize! (sprintf "%%-14s" "%s )-1,"))
        =]z[=% name (string-capitalize! (sprintf "%%-10s" "%s,"))
        =]doDir_[=
  IF (exist? "dummy") =]IGNORE,   [=
  ELSE  =][=% name (string-downcase! (sprintf "%%-10s" "%s,")) =][=
  ENDIF =]0 }[=
ENDFOR directive=] };

/*
 *  This text has been extracted from [=`echo ${srcdir}/schemedef.scm`=]
 */
tSCC zSchemeInit[] =
[= (kr-string (shell

"sed \"s/AUTOGEN_VERSION/${AG_VERSION}/;s/^[ \t]*//
/^\\(;\\|$\\)/d\" ${srcdir}/schemedef.scm" ))

=];
#endif /* DEFINING */
#endif /* [=(. header-guard)=] */[= #

end of directive.tpl  =]
