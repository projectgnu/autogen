[= AutoGen5 Template -*- Mode: text -*-

# $Id: optmain.tpl,v 4.7 2005/02/04 05:10:07 bkorb Exp $

# Automated Options copyright 1992-2005 Bruce Korb

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD GUILE MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-guile-main

=][=
(tpl-file-line extract-fmt)
=]
int    original_argc;
char** original_argv;

static void
inner_main( int argc, char** argv )
{
    original_argc = argc;
    original_argv = argv;

    {
        int ct = optionProcess( &[=(. pname)=]Options, argc, argv );
        char** new_argv = (char**)malloc( (argc - ct + 2)*sizeof(char*) );

        if (new_argv == NULL) {
            fputs( _("[=(. pname)=] cannot allocate new argv\n"), stderr );
            exit( EXIT_FAILURE );
        }

        /*
         *  argc will be reduced by one less than the count returned
         *  by optionProcess.  That count includes the program name,
         *  but we are adding the program name back in (plus a NULL ptr).
         */
        argc -= (ct-1);
        new_argv[0] = argv[0];

        /*
         *  Copy the argument pointers, plus the terminating NULL ptr.
         */
        memcpy( new_argv+1, argv + ct, argc * sizeof( char* ));
        argv = new_argv;
    }[=

  IF (> (len "guile-main") 0)  =][=

(def-file-line "copyright.text" extract-fmt) =]
[= guile-main =]
    exit( EXIT_SUCCESS );[=
  ELSE  =]

    export_options_to_guile( &[=(. pname)=]Options );
    scm_shell( argc, argv );[=
  ENDIF =]
}

int
main( int    argc,
      char** argv )
{[=
  (if (exist? "before-guile-boot")
    (string-append (def-file-line "before-guile-boot" extract-fmt)
       (get "before-guile-boot") ) )
=]
    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return 0;
}
[=

ENDDEF build-guile-main

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD TEST MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-test-main

=][=
(tpl-file-line extract-fmt)
=]
#if defined([=(. main-guard)=]) /* TEST MAIN PROCEDURE: */

int
main( int argc, char** argv )
{
    int res = EXIT_SUCCESS;[=

  IF (= (get "test-main") "putShellParse") =]
    extern tOptions  genshelloptOptions;
    extern void      putShellParse( tOptions* );
    extern tOptions* pShellParseOptions;

    /*
     *  Stash a pointer to the options we are generating.
     *  `genshellUsage()' will use it.
     */
    pShellParseOptions = &[=(. pname)=]Options;
    (void)optionProcess( &genshelloptOptions, argc, argv );
    putShellParse( &[=(. pname)=]Options );[=

  ELIF (exist? "main-text") =][=
    IF (not (exist? "option-code")) =]
    {
        int ct = optionProcess( &[=(. pname)=]Options, argc, argv );
        argc -= ct;
        argv += ct;
    }[=
    ELSE            =][=
      (def-file-line "option-code" extract-fmt) =][=
      option-code   =][=
    ENDIF           =][=

  (def-file-line "main-text" extract-fmt) =][=
  main-text         =][=

  ELSE=]
    (void)optionProcess( &[=(. pname)=]Options, argc, argv );[=

    (set! opt-name (get "test-main"))
    (if (<= (string-length opt-name) 3)
        (set! opt-name "putBourneShell")) =]
    {
        void [= (. opt-name) =]( tOptions* );
        [= (. opt-name) =]( &[=(. pname)=]Options );
    }[=
  ENDIF=]
    return res;
}
#endif  /* defined [= (. main-guard) =] */[=

ENDDEF  build-test-main

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD FOR-EACH MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE for-each-main            =][=

  (if (not (==* (get "argument") "[" ))
      (error "command line arguments must be optional for a 'for-each' main"))

  (if (not (exist? "handler-proc"))
      (error "'for-each' mains require a handler proc") )

=][=
(define handler-arg-type "")
(tpl-file-line extract-fmt)
=]
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>

static char*
trim_input_line( char* pz_s )
{
    char* pz_e = pz_s + strlen( pz_s );
    while ((pz_e > pz_s) && isspace( pz_e[-1] ))  pz_e--;
    *pz_e = '\0';
    while (isspace( *pz_s ))  pz_s++;

    switch (*pz_s) {
    case '\0':
    case '[= ?% comment-char "%s" "#" =]':
        return NULL;
    default:
        return pz_s;
    }
}[=

CASE handler-type =][=
=*  name         =][= (set! handler-arg-type "const char* pz_fname")
                      (define handler-proc "validate_fname")       =][=
=*  file         =][=
   (set! handler-arg-type "const char* pz_fname, FILE* entry_fp")
                      (define handler-proc "validate_fname")       =][=
*=* text         =][=
   (set! handler-arg-type
         "const char* pz_fname, char* pz_file_text, size_t text_size")
                      (define handler-proc "validate_fname")       =][=
!E               =][= (set! handler-arg-type "const char* pz_entry")
                      (define handler-proc (get "handler-proc"))       =][=
*                =][= (error) =][=
ESAC             =][=

IF (set! tmp-text (string-append (get "handler-proc") "-code"))
   (exist? tmp-text) =]

static int
[= handler-proc =]( [=(. handler-arg-type)=] )
{
    int res = 0;[=
   (string-append
      (def-file-line tmp-text extract-fmt)
      (get tmp-text) ) =]
    return res;
}[=

ELSE

=]

extern int [= handler-proc =]( [=(. handler-arg-type)=] );[=

ENDIF

=][=
(tpl-file-line extract-fmt)
=][=

IF (exist? "handler-type")  =]

static int
validate_fname( const char* pz_fname )
{
    const char* pz_fs_err =
        _("fs error %d (%s) %s-ing %s\n");[=

  IF (*=* (get "handler-type") "text") =]
    char*  file_text;
    size_t text_size;
    int    res;[=
  ENDIF =]

    {
        struct stat sb;
        if (stat( pz_fname, &sb ) < 0) {
            fprintf( stderr, pz_fs_err, errno, strerror(errno), "stat",
                     pz_fname );
            return 1;
        }[=

  IF (*=* (get "handler-type") "text") =]

        if (! S_ISREG(sb.st_mode)) {
            fprintf( stderr, pz_fs_err, EINVAL, strerror(EINVAL),
                     "not regular file:", pz_fname );
            return 1;
        }[=

    IF (=* (get "handler-type") "some-text") =]

        if (sb.st_size == 0) {
            fprintf( stderr, pz_fs_err, EINVAL, strerror(EINVAL),
                     "empty file:", pz_fname );
            return 1;
        }[=

    ENDIF  =]

        text_size = sb.st_size;[=

  ENDIF

=]
    }[=

CASE handler-type =][=
=*  name         =][=
    (tpl-file-line extract-fmt)
    =]

    return [= handler-proc =](pz_fname);[=

=*  file         =][=
    (tpl-file-line extract-fmt)
    =]
    {
        int res;
        FILE* fp = fopen(pz_fname, "[=
        (shellf "echo '%s' | sed 's/.*-//'"
                (get "handler-type")) =]");
        if (fp == NULL) {
            fprintf( stderr, pz_fs_err, errno, strerror(errno), "fopen",
                     pz_fname );
            return 1;
        }
        res = [= handler-proc =](pz_fname, fp);
        fclose(fp);
        return res;
    }[=

*=*  text        =][=
    (tpl-file-line extract-fmt)
    =]
    file_text = malloc( text_size + 1 );
    if (file_text == NULL) {
        fprintf(stderr, _("cannot allocate %d bytes for %s file text\n"),
                text_size+1, pz_fname);
        exit( EXIT_FAILURE );
    }

    {
        char*   pz = file_text;
        size_t  sz = text_size;
        int     fd = open(pz_fname, O_RDONLY);
        int     try_ct = 0;

        if (fd < 0) {
            fprintf( stderr, pz_fs_err, errno, strerror(errno), "open",
                     pz_fname );
            return 1;
        }

        while (sz > 0) {
            ssize_t rd_ct = read( fd, pz, sz );
            /*
             *  a read count of zero is theoretically okay, but we've already
             *  checked the file size, so we shoud be reading more.
             *  For us, a count of zero is an error.
             */
            if (rd_ct <= 0) {
                /*
                 * Try retriable errors up to 10 times.  Then bomb out.
                 */
                if (  ((errno == EAGAIN) || (errno == EINTR))
                   && (++try_ct < 10)  )
                    continue;

                fprintf( stderr, pz_fs_err, errno, strerror(errno), "read",
                         pz_fname );
                exit( EXIT_FAILURE );
            }
            pz += rd_ct;
            sz -= rd_ct;
        }
        close(fd);
    }

    /*
     *  Just in case it is a text file, we have an extra byte to NUL
     *  terminate the thing.
     */
    file_text[ text_size ] = '\0';
    res = [= handler-proc =](pz_fname, file_text, text_size);
    free(file_text);
    return res;[=
ESAC             =]
}[=

ENDIF handler-type exists

=][=
(tpl-file-line extract-fmt)
=]

int
main( int argc, char** argv )
{
    int res = 0;
    {
        int ct = optionProcess( &[=(. pname)=]Options, argc, argv );
        argc -= ct;
        argv += ct;
    }[=

    (def-file-line "main-init" extract-fmt) =][=
    main-init =][=
    (tpl-file-line extract-fmt)

=]
    /*
     *  Input list from command line
     */
    if (argc > 0) {
        do  {
            res |= [= (. handler-proc) =]( *(argv++) );
        } while (--argc > 0);
    }

    /*
     *  Input list from tty input
     */
    else if (isatty( STDIN_FILENO )) {
        fputs( _("[=(. prog-name)=] ERROR: input list is a tty\n"), stderr );
        [= (. UP-prefix) =]USAGE( EXIT_FAILURE );
        /* NOTREACHED */
    }

    /*
     *  Input list from a pipe or file or some such
     */
    else {
        int in_ct   = 0;
        int pg_size = getpagesize();
        char* buf   = malloc( pg_size );
        if (buf == NULL) {
            fputs( _("[=(. prog-name)
                   =] ERROR: no memory for input list\n"), stderr );
            return EXIT_FAILURE;
        }

        for (;;) {
            char* pz = fgets( buf, pg_size, stdin );
            if (pz == NULL)
                break;

            pz = trim_input_line( pz );
            if (pz != NULL) {
                 res |= [= (. handler-proc) =]( pz );
                 in_ct++;
            }
        }

        if (in_ct == 0)
            fputs( _("[=(. prog-name)
                   =] Warning:  no input lines were read\n"), stderr );
        free( buf );
    }[=

    (def-file-line "main-fini" extract-fmt) =][=
    main-fini =][=
    (tpl-file-line extract-fmt)

=]

    return res;
}[=

ENDDEF  for-each-main

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-main               =][= FOR main[] =][=

 CASE main-type                 =][=
  == guile                      =][=
     build-guile-main           =][=

  == shell-process              =][=
     INVOKE build-test-main  test-main = "putBourneShell" =][=

  == shell-parser               =][=
     INVOKE build-test-main  test-main = "putShellParse"  =][=

  == main                       =][=
     INVOKE build-test-main     =][=

  == include                    =][=
     INCLUDE tpl                =][=

  == invoke                     =][=
     INVOKE (get "func")        =][=

  == for-each                   =][=
     INVOKE for-each-main       =][=

  *                             =][=
     (error (sprintf "unknown/invalid main-type: '%s'" (get "main-type"))) =][=

  ESAC =][= ENDFOR =][=

ENDDEF build-main

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DECLARE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE declare-option-callbacks

   This is the test for whether or not to emit callback handling code:

=]
/*
 *  Declare option callback procedures
 */[=
  (define undef-proc-names "")
  (define extern-proc-list (string-append

    "doPagedUsage\n"

    (if (exist? "version") "doVersion\n" "")
  ) )

  (define extern-test-list (string-append

    "doPagedUsage\n"

    (if (exist? "version") "doVersionStderr\n" "")
  ) )

  (define emit-decl-list (lambda(txt-var is-extern)

    (set! txt-var (shellf "
      (egrep -v '^%s$' | sort -u | \
      sed 's@$@,@;$s@,$@;@' ) <<_EOProcs_\n%s_EOProcs_"
          (if is-extern "NULL" "(NULL|stackOptArg|unstackOptArg)")
          txt-var ))

    (shellf (if (< (string-length txt-var) 72)
                "f='%s' ; echo \"   \" $f"
                "${CLexe} --spread=1 -I4 <<_EOProcs_\n%s\n_EOProcs_" )
          txt-var )  ))

  (define static-proc-list "doUsageOpt\n")
  (define static-test-list static-proc-list)   =][=

  FOR    flag   =][=

    (set! flg-name (get "name"))
    (if (hash-ref have-cb-procs flg-name)
        (begin
          (if make-test-main (begin
              (set! tmp-val (hash-ref test-proc-name flg-name))
              (if (hash-ref is-ext-cb-proc flg-name)
                  (set! extern-test-list (string-append extern-test-list
                        tmp-val "\n" ))

                  (set! static-test-list (string-append static-test-list
                        tmp-val "\n" ))
              )
          )   )

          (set! tmp-val (hash-ref cb-proc-name flg-name))

          (if (hash-ref is-ext-cb-proc flg-name)
              (set! extern-proc-list (string-append extern-proc-list
                    (hash-ref cb-proc-name flg-name) "\n" ))

              (set! static-proc-list (string-append static-proc-list
                    (hash-ref cb-proc-name flg-name) "\n" ))
          )

          (if (exist? "ifdef") (begin
              (emit (sprintf "\n#ifndef %s\n#define %s doUsageOpt\n#endif"
                         (get "ifdef") tmp-val ))
              (set! undef-proc-names (string-append undef-proc-names
                 (sprintf
                     "\n#ifndef %1$s\n#undef  %2$s\n#define %2$s NULL\n#endif"
                     (get "ifdef") tmp-val )  ))
          )   )

          (if (exist? "ifndef") (begin
              (emit (sprintf "\n#ifdef %s\n#define %s doUsageOpt\n#endif"
                             (get "ifdef") tmp-val ))
              (set! undef-proc-names (string-append undef-proc-names
                 (sprintf
                     "\n#ifdef %1$s\n#undef  %2$s\n#define %2$s NULL\n#endif"
                     (get "ifndef") tmp-val )  ))
          )   )
    )   ) ""    =][=

  ENDFOR flag   =][=

  IF (. make-test-main)

=][=
(tpl-file-line extract-fmt)
=]
#if defined([=(. main-guard)=])
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */
extern tOptProc
[=

(emit-decl-list extern-test-list #t)  =][=

    IF (> (string-length static-test-list) 0)

=]
static tOptProc
[=(emit-decl-list static-test-list #f)=][=

    ENDIF have static test procs  =][=
    (set! static-test-list "")    =][=
    FOR     flag          =][=

      (set! flg-name (get "name"))
      (if (not (= (hash-ref cb-proc-name  flg-name)
                 (hash-ref test-proc-name flg-name)))
          (set! static-test-list (string-append static-test-list
                "#define " (up-c-name "name") "_OPT_PROC "
                       (hash-ref test-proc-name flg-name) "\n"))  )
    =][=
    ENDFOR  flag          =][=

    IF (> (string-length static-test-list) 0) =]

/*
 *  #define map the "normal" callout procs to the test ones...
 */
[= (. static-test-list) =][=

    ENDIF  have some #define mappings

=]

#else /* NOT defined [=(. main-guard)=] */
/*
 *  When not under test, there are different procs to use
 */[=

  ENDIF make-test-main

=]
extern tOptProc
[=(emit-decl-list extern-proc-list #t)=][=

  IF (> (string-length static-proc-list) 0)

=]
static tOptProc
[=(emit-decl-list static-proc-list #f)=][=

  ENDIF have static test procs  =][=
  (set! static-proc-list "")    =][=
  FOR     flag           =][=

      (set! flg-name (get "name"))
      (if (not (= (hash-ref cb-proc-name  flg-name)
                 (hash-ref test-proc-name flg-name)))
          (set! static-proc-list (string-append static-proc-list
                "#define " (up-c-name "name") "_OPT_PROC "
                       (hash-ref cb-proc-name flg-name) "\n"))  )
  =][=
  ENDFOR  flag            =][=

  IF (> (string-length static-proc-list) 0) =]

/*
 *  #define map the "normal" callout procs
 */
[= (. static-proc-list) =][=

  ENDIF  have some #define mappings

=][=

  IF (. make-test-main)   =][=

    FOR     flag          =][=
      IF (not (= (hash-ref cb-proc-name   flg-name)
                 (hash-ref test-proc-name flg-name))) =]
#define [=(up-c-name "name")=]_OPT_PROC [=(hash-ref cb-proc-name flg-name)=][=
      ENDIF               =][=
    ENDFOR  flag          =]
#endif /* defined([=(. main-guard)=]) */[=

  ENDIF (. make-test-main)      =][=

(. undef-proc-names)  =][=

ENDDEF declare-option-callbacks

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DEFINE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE callback-proc-header     =]

/* * * * * * *
 *
 *   For the [=name=] option[=

  IF (exist? "ifdef")

=], when [= ifdef =] is #define-d.
 */
#ifdef [= ifdef                 =][=
  ELIF (exist? "ifndef")

=], when [= ifdef =] is *not* #define-d.
 */
#ifndef [= ifndef               =][=
  ELSE                          =].
 */[=

  ENDIF ifdef / ifndef

=]
static void
doOpt[=(. cap-name) =](
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
{
[=

ENDDEF   callback-proc-header

# # # # # # # # # # # # # # # # =][=

DEFINE range-option-code

=][=

(if (not (=* (get "arg-type") "num"))
    (error (string-append "range option " low-name " is not numeric")) )

=]    static const struct {const int rmin, rmax;} rng[ [=
      (count "arg-range")  =] ] = {
[=(out-push-new)      =][=
  FOR arg-range ",\n" =][=
    CASE arg-range    =][=
      *==    "->"     =][=
             (shellf "f=`echo '%s'|sed 's,->$,,'`
                     echo \"{ $f, INT_MAX }\"" (get "arg-range")) =][=

      ==*    "->"     =][=
             (shellf "f=`echo '%s'|sed 's,^->,,'`
                     echo \"{ INT_MIN, $f }\"" (get "arg-range")) =][=

      *==*   "->"     =][=
             (shellf "f=`echo '%s'|sed 's/->/, /'`
                     echo \"{ $f }\"" (get "arg-range")) =][=

      ~~ -{0,1}[0-9]+ =]{ [=arg-range=], INT_MIN }[=

      *  =][= (error (string-append "Invalid range spec:  ``"
              (get "arg-range") "''" ))  =][=

    ESAC arg-range    =][=
  ENDFOR =][=
  (shellf "${CLexe} -I8 --spread=2 <<_EOF_\n%s\n_EOF_"
          (out-pop #t)) =] };
    long val;
    int ix;
    tCC* pzIndent = "\t\t\t\t  ";
    extern FILE* option_usage_fp;

    if (pOptDesc == NULL) /* usage is requesting range list
                             option_usage_fp has already been set */
        goto emit_ranges;

    val = atoi( pOptDesc->pzLastArg );
    for (ix = 0; ix < [=(count "arg-range")=]; ix++) {
        if (val < rng[ix].rmin)
            continue;  /* ranges need not be ordered. */
        if (val == rng[ix].rmin)
            goto valid_return;
        if (rng[ix].rmax == INT_MIN)
            continue;
        if (val <= rng[ix].rmax)
            goto valid_return;
    }

    option_usage_fp = stderr;
    fprintf( stderr, _("%s error:  %s option value ``%s''is out of range.\n"),
             pOptions->pzProgName, pOptDesc->pz_Name, pOptDesc->pzLastArg );
    pzIndent = "\t";

  emit_ranges:[=


  IF (> (count "arg-range") 1) =]
    fprintf( option_usage_fp, _("%sit must lie in one of the ranges:\n"),
             pzIndent );
    for ( ix=0;; ) {
        if (rng[ix].rmax == INT_MIN)
             fprintf(option_usage_fp, _("%s%d exactly"), pzIndent,
                     rng[ix].rmin);
        else fprintf(option_usage_fp, _("%s%d to %d"), pzIndent,
                      rng[ix].rmin, rng[ix].rmax );
        if (++ix >= [=(count "arg-range")=])
            break;
        fputs( _(", or\n"), option_usage_fp );
    }

    fputc( '\n', option_usage_fp );[=

  ELIF (*==* (get "arg-range") "->")  =]
    fprintf( option_usage_fp, _("%sit must lie in the range: %d to %d\n"),
             pzIndent, rng[0].rmin, rng[0].rmax );[=

  ELSE  =]
    fprintf( option_usage_fp, _("%sit must be: %d exactly\n"),
             pzIndent, rng[0].rmin );[=

  ENDIF =]
    if (pOptDesc == NULL)
        return;

    [=(. UP-prefix)=]USAGE( EXIT_FAILURE );
    /* NOTREACHED */
    return;

  valid_return:
    pOptDesc->pzLastArg = (char*)val;[=

ENDDEF  range-option-code

# # # # # # # # # # # # # # # # =][=

DEFINE define-option-callbacks  =][=

  FOR  flag  =][=

    (set! UP-name    (up-c-name "name"))
    (set! cap-name   (string-capitalize UP-name))
    (set! low-name   (string-downcase UP-name))      =][=

    IF (or (exist? "flag-code")
           (exist? "extract-code")
           (exist? "arg-range") ) =][=

      IF (. make-test-main) =]

#if ! defined([=(. main-guard)=])[=

      ENDIF =][=

      invoke callback-proc-header  =][=

      IF (exist? "flag-code")      =][=

         flag-code                 =][=

      ELIF (exist? "extract-code") =][=
         (extract (string-append (base-name) ".c.save") (string-append
                  "/*  %s =-= " cap-name " Opt Code =-= %s */"))
         =][=

      ELIF (exist? "arg-range")    =][=
         range-option-code         =][=

      ENDIF =]
}[=

  IF (exist? "ifdef")           =]
#endif /* defined [= ifdef      =] */[=
  ELIF (exist? "ifndef")        =]
#endif /* ! defined [= ifndef   =] */[=
  ENDIF ifdef / ifndef          =][=

      IF (. make-test-main) =]

#endif /* ! defined [= (. main-guard) =] */[=

      ENDIF =][=


    ELIF (=* (get "arg-type") "key") =][=

      invoke callback-proc-header  =][=
      IF (not (exist? "arg-default"))
=]    tSCC zDef[2] = { 0x7F, 0 };
[=    ENDIF

=]    tSCC* azNames[] = {[=
      IF (not (exist? "arg-default")) =] zDef,[=
      ENDIF  =]
[=(shellf
  "${CLexe} -I8 --spread=2 --sep=',' -f'\"%%s\"' <<_EOF_\n%s\n_EOF_\n"
          (join "\n" (stack "keyword")) ) =]
    };
[=

      IF (exist? "arg-optional") =]
    if (((tUL)pOptions > 0x0FUL) && (pOptDesc->pzLastArg == NULL))
        pOptDesc->pzLastArg = (char*)[=
         (string-append UP-name "_"    (if (> (len "arg-optional") 0)
            (up-c-name "arg-optional") (if (exist? "arg-default")
            (up-c-name "arg-default")
            "UNDEFINED"  ))) =];
    else[=
      ENDIF =]
    pOptDesc->pzLastArg =
        optionEnumerationVal( pOptions, pOptDesc, azNames, [=
        (+ (count "keyword") (if (exist? "arg-default") 0 1)) =] );
}[=


    ELIF (=* (get "arg-type") "set") =][=

      invoke callback-proc-header
=]    tSCC* azNames[] = {
[=(shellf
  "${CLexe} -I8 --spread=2 --sep=',' -f'\"%%s\"' <<_EOF_\n%s\n_EOF_\n"
          (join "\n" (stack "keyword")) )=]
    };
    optionSetMembers( pOptions, pOptDesc, azNames, [=
        (count "keyword") =] );
}[=


    ENDIF       =][=
  ENDFOR flag   =]
[=

ENDDEF define-option-callbacks

=]
