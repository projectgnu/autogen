[= AutoGen5 template -*- Mode: C -*-
# $Id: directive.tpl,v 3.9 2004/08/31 02:35:14 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h =]
[=(dne " *  " "/*  ")=]
 *
 *  copyright 1992-2004 Bruce Korb
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
#define SCHEME_INIT_FILE [= (c-string (out-name)) =]
#define SCHEME_INIT_LINE [= # (+ (out-line) 2) =] 131
tSCC zSchemeInit[] =
[= (kr-string (shell

"sed -e \"s/AUTOGEN_VERSION/${AG_VERSION}/;s/^[ \t]*//\" \\
     -e '/^;/d;/^$/d' ${srcdir}/schemedef.scm" ))

=]; /* for emacs: " */

#ifdef DAEMON_ENABLED
typedef struct inet_family_map_s {
    const char*     pz_name;
    unsigned short  nm_len;
    unsigned short  family;
} inet_family_map_t;

inet_family_map_t inet_family_map[] = {
[=`

find /usr/include -follow -name socket.h | \
xargs egrep '^#define[ \t]+AF_[A-Z0-9]+[ \t]+[0-9]' | \
sed 's,^.*#define[ \t]*AF_,,;s,[ \t].*,,' | \
 while read f
 do
   case $f in
   MAX ) echo '    { NULL, 0, 0 } };'
         break ;;
   esac

   g=\`echo $f | tr '[A-Z]' '[a-z]'\`:
   echo "    { \\\"${g}\\\", ${#g}, AF_${f} },"
 done

`=]
#endif /* DAEMON_ENABLED */
#endif /* DEFINING */
#endif /* [=(. header-guard)=] */[= #

end of directive.tpl  =]
