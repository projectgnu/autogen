[= autogen template -*- Mode: C -*-
# $Id: directive.tpl,v 1.1.1.1 1999/10/14 00:33:53 bruce Exp $
h =]
/*
[=_eval "# *  " _DNE=]
 *
 *  copyright 1992-1999 Bruce Korb
 *
[=_eval AutoGen "# *  " _gpl=]
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
_FOR directive =][=
  _IF dummy _exist ! =]
static tDirProc doDir_[=name=];[=
  _ENDIF=][=
/directive =]

/*
 *  Define the constant string names for each directive
 */[=
_FOR directive =]
static const char z[=name _cap "#[]" + "#%-12s" _printf
       =] = [=name _down _str=];[=
/directive
=]

/*
 *  Enumerate the directives
 */
typedef enum {[=
_FOR directive , =]
    DIR_[=name _up=][=
/directive=]
} teDirectives;

#define DIRECTIVE_CT  [=_eval directive _hilim 1 +=]
static tDirTable dirTable[ DIRECTIVE_CT ] = {[=
_FOR directive , =]
    { sizeof( z[=name _cap "# )-1," + "#%-14s" _printf
       =]z[=name _cap #, + "#%-10s" _printf
       =]doDir_[=
  _IF dummy _exist =]IGNORE,   [=
  _ELSE =][= name #, + "#%-10s" _printf =][=
  _ENDIF=]0 }[=
/directive=] };

#endif /* AUTOGEN_DIRECTIVE_H */[=

end of directive.tpl  =]
