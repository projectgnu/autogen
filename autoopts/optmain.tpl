[= AutoGen5 Template -*- Mode: text -*-

# $Id: optmain.tpl,v 3.8 2003/04/04 04:44:54 bkorb Exp $

# Automated Options copyright 1992-2003 Bruce Korb

=]
[=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD GUILE MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-guile-main

=]
int    original_argc;
char** original_argv;


    static void
#ifdef __cplusplus
inner_main( int argc, char** argv )
#else
inner_main( argc, argv )
      int    argc;
      char** argv;
#endif
{
    original_argc = argc;
    original_argv = argv;

    {
        int ct = optionProcess( &[=(. pname)=]Options, argc, argv );
        char** new_argv = (char**)malloc( (argc - ct + 2)*sizeof(char*) );

        if (new_argv == NULL) {
            fputs( "[=(. pname)=] cannot allocate new argv\n", stderr );
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
    }
[=
  IF (> (len "guile-main") 0)  =]
    [=guile-main=]
    exit( EXIT_SUCCESS );[=
  ELSE  =]
    export_options_to_guile( &[=(. pname)=]Options );
    scm_shell( argc, argv );[=
  ENDIF =]
}

int
#ifdef __cplusplus
main( int    argc,
      char** argv )
#else
main( argc, argv )
      int    argc;
      char** argv;
#endif
{[=
    % before-guile-boot "\n    %s\n"
 =]
    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return 0;
}
[=

ENDDEF

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   BUILD TEST MAIN

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE build-test-main

=]
#if defined( [= (. main-guard) =] )

int
main( int argc, char** argv )
{[=

  IF (= (get "test-main") "putShellParse") =]
    extern tOptions  genshelloptOptions;
    extern void      putShellParse( tOptions* );
    extern tOptions* pShellParseOptions;

    /*
     *  Stash a pointer to the options we are generating.
     *  `genshellUsage()' will use it.
     */
    pShellParseOptions = &[=prog-name=]Options;
    (void)optionProcess( &genshelloptOptions, argc, argv );
    putShellParse( &[=prog-name=]Options );[=

  ELIF
      ;;  Also check to see if the user supplies all the code for main()
      (exist? "main-text") =]
[= main-text =][=

  ELSE=]
    (void)optionProcess( &[=prog-name=]Options, argc, argv );[=

    IF  ;;  the user is specifying a routine to call, then call that procedure

        (> (string-length (get "test-main")) 3) =]

    {
        void [=test-main=]( tOptions* );
        [=test-main=]( &[=prog-name=]Options );
    }[=

   ELSE  Call our library procedure for emitting shell results
    =]
    putBourneShell( &[=prog-name=]Options );[=

    ENDIF =][=
  ENDIF=]
    return EXIT_SUCCESS;
}
#endif  /* defined [= (. main-guard) =] */[=

ENDDEF  =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DECLARE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE declare-option-callbacks      =][=

   #  For test builds, no need to call option procs  =][=

  IF (exist? "test-main")            =]
#if ! defined( [=(. main-guard)=] )[=
  ENDIF

  =]
/*
 *  Procedures to call when option(s) are encountered
 */[=


  FOR flag                           =][=
    (set-flag-names)                 =][=

    CASE arg-type                    =][=
    =*  key                          =]
static tOptProc doOpt[=(. cap-name)  =];[=
    =*  bool                         =][=
    =*  num                          =][=
      IF (exist? "arg-range")        =]
static tOptProc doOpt[=(. cap-name)  =];[=
      ENDIF                          =][=
    *                                =][=
      IF (exist? "call-proc")        =]
extern tOptProc [=(get "call-proc")  =];[=

      ELIF (or (exist? "extract-code")
               (exist? "flag-code")) =]
static tOptProc doOpt[=(. cap-name)  =];[=

      ENDIF                          =][=
    ESAC                             =][=
  ENDFOR                             =][=
 
  IF (exist? "test-main")            =][=

     # "A test environment is to be generated" =]

#else /* *is*  defined( [=(. main-guard)=] ) */
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */[=
    FOR flag            =][=
    (set! cap-name (string->c-name! (string-capitalize! (get "name"))) ) =][=

      IF (exist? "call-proc") =]
#define [=(get "call-proc")   =] [=
          IF (~ (get "max") "1{0,1}")
                        =]NULL[=
          ELSE          =]stackOptArg[=
          ENDIF         =][=

      ELIF (or (exist? "flag-code")
               (exist? "extract-code")
               (exist? "arg-range")) =]
#define doOpt[=(. cap-name)   =] [=
          IF (~ (get "max") "1{0,1}")
                        =]NULL[=
          ELSE          =]stackOptArg[=
          ENDIF         =][=

      ELIF (=* (get "arg-type") "key")  =]
static tOptProc doOpt[=(. cap-name)  =];[=
      ENDIF             =][=

    ENDFOR flag         =]
#endif /* defined( [=(. main-guard)=] ) */[=
  ENDIF (exist? "test-main") =]
[=

ENDDEF

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DEFINE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE callback-proc-header     =]

/* * * * * * *
 *
 *   For the "[=(string-capitalize! (get "name"))=] Option".
 */
static void
#ifdef __cplusplus
doOpt[=(. cap-name) =](
    tOptions*   pOptions,
    tOptDesc*   pOptDesc )
#else
doOpt[=(. cap-name) =]( pOptions, pOptDesc )
    tOptions*   pOptions;
    tOptDesc*   pOptDesc;
#endif
{
[=

ENDDEF                          =][=

# # # # # # # # # # # # # # # # =][=

DEFINE range-option-code

=][=

(if (not (=* (get "arg-type") "num"))
    (error (string-append "range option " low-name " is not numeric")) )

=]    static const struct {const int rmin, rmax;} rng[ [=(count "arg-range")=] ] = {
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
  (shellf "${COLUMNS_EXE} -I8 --spread=2 <<_EOF_\n%s\n_EOF_"
          (out-pop #t)) =] };
    int val, ix;
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
    fprintf( stderr, "%s error:  %s option value ``%s''is out of range.\n",
             pOptions->pzProgName, pOptDesc->pz_Name, pOptDesc->pzLastArg );
    pzIndent = "\t";

  emit_ranges:[=


  IF (> (count "arg-range") 1) =]
    fprintf( option_usage_fp, "%sit must lie in one of the ranges:\n", pzIndent );
    for ( ix=0;; ) {
        if (rng[ix].rmax == INT_MIN)
             fprintf( option_usage_fp, "%s%d exactly", pzIndent, rng[ix].rmin );
        else fprintf( option_usage_fp, "%s%d to %d", pzIndent,
                      rng[ix].rmin, rng[ix].rmax );
        if (++ix >= [=(count "arg-range")=])
            break;
        fputs( ", or\n", option_usage_fp );
    }

    fputc( '\n', option_usage_fp );[=

  ELIF (*==* (get "arg-range") "->")  =]
    fprintf( option_usage_fp, "%sit must lie in the range: %d to %d\n",
             pzIndent, rng[0].rmin, rng[0].rmax );[=

  ELSE  =]
    fprintf( option_usage_fp, "%sit must be: %d exactly\n",
             pzIndent, rng[0].rmin );[=

  ENDIF =]
    if (pOptDesc == NULL)
        return;

    [=(. UP-prefix)=]USAGE( EXIT_FAILURE );
    /* NOTREACHED */

  valid_return:
    pOptDesc->pzLastArg = (char*)val;[=

ENDDEF                          =][=

# # # # # # # # # # # # # # # # =][=

DEFINE define-option-callbacks  =][=

  FOR  flag  =][=

    (set! UP-name    (string->c-name! (string-upcase! (get "name"))) )
    (set! cap-name   (string-capitalize UP-name))
    (set! low-name   (string-downcase UP-name))      =][=

    IF (or (exist? "flag-code")
           (exist? "extract-code")
           (exist? "arg-range") ) =][=

      IF (exist? "test-main") =]

#if ! defined( [= (. main-guard) =] )[=

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

  IF (exist? "test-main") =]

#endif /* ! defined [= (. main-guard) =] */[=

  ENDIF =][=


    ELIF (=* (get "arg-type") "key") =][=

      invoke callback-proc-header  =][=
      IF (not (exist? "arg-default"))
=]    tSCC zDef[2] = { 0x7F, 0 };
[=    ENDIF =][=

    IF (> (len "arg-optional") 0) =]
    te_[=(string-append Cap-prefix cap-name)=] def_val = [=
       IF (not (=* (get "arg-optional") low-name))
             =][=(. UP-name)=]_[=
       ENDIF =][=(string-upcase! (string->c-name! (get "arg-optional")))
             =];[=
    ENDIF
=]    tSCC* az_names[] = {[=
      IF (not (exist? "arg-default")) =] zDef,[=
      ENDIF  =]
[=(shellf
  "${COLUMNS_EXE} -I8 --spread=2 --sep=',' -f'\"%%s\"' <<_EOF_\n%s\n_EOF_\n"
          (join "\n" (stack "keyword")) )=]
    };
[=

    IF (> (len "arg-optional") 0) =]
    if (((tUL)pOptions > 0x0FUL) && (pOptDesc->pzLastArg == NULL))
        pOptDesc->pzLastArg = (char*)def_val;
    else[=
    ENDIF =]
    pOptDesc->pzLastArg =
        optionEnumerationVal( pOptions, pOptDesc, az_names, [=
        (+ (count "keyword") (if (exist? "arg-default") 0 1)) =] );
}[=
    ENDIF       =][=
  ENDFOR flag   =]
[=

ENDDEF

=]
