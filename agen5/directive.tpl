[= AutoGen5 template -*- Mode: C -*-
# $Id: directive.tpl,v 1.4 2001/10/13 18:48:48 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h =]
/*
[=(dne " *  ")=]
 *
 *  copyright 1992-1999 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 */
#ifndef AUTOGEN_DIRECTIVE_H
#define AUTOGEN_DIRECTIVE_H

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
FOR directive =][=
  IF (not (exist? "dummy")) =]
static tDirProc doDir_[=name=];[=
  ENDIF=][=
ENDFOR directive =]

/*
 *  Define the constant string names for each directive
 */[=
FOR directive =]
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

#define DIRECTIVE_CT  [= (+ ( high-lim "directive") 1) =]
static tDirTable dirTable[ DIRECTIVE_CT ] = {[=
FOR directive , =]
    { sizeof( z[=% name (string-capitalize! (sprintf "%%-14s" "%s )-1,"))
       =]z[=% name (string-capitalize! (sprintf "%%-10s" "%s,"))
       =]doDir_[=
  IF (exist? "dummy") =]IGNORE,   [=
  ELSE =][=% name (string-downcase! (sprintf "%%-10s" "%s,")) =][=
  ENDIF=]0 }[=
ENDFOR directive=] };

#endif /* AUTOGEN_DIRECTIVE_H */[= #

end of directive.tpl  =]
