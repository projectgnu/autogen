[= AutoGen5 Template -*- Mode: text -*-

# $Id: optmain.tpl,v 2.11 2000/10/29 01:43:32 bkorb Exp $

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
inner_main( int argc, char** argv )
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
main( int    argc,
      char** argv )
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
#if defined( TEST_[= (. pname-up) =]_OPTS )

    int
main( int argc, char** argv )
{[=

  IF (= (get "test_main") "putShellParse") =]
    extern tOptions  genshelloptOptions;
    extern void      putShellParse( tOptions* );
    extern tOptions* pShellParseOptions;

    /*
     *  Stash a pointer to the options we are generating.
     *  `genshellUsage()' will use it.
     */
    pShellParseOptions = &[=prog_name=]Options;
    (void)optionProcess( &genshelloptOptions, argc, argv );
    putShellParse( &[=prog_name=]Options );[=

  ELSE=]
    (void)optionProcess( &[=prog_name=]Options, argc, argv );[=
    IF (> (string-length (get "test_main")) 3) =]

    {
        void [=test_main=]( tOptions* );
        [=test_main=]( &[=prog_name=]Options );
    }
[=  ELSE=]
    putBourneShell( &[=prog_name=]Options );[=

    ENDIF =][=
  ENDIF=]
    return EXIT_SUCCESS;
}
#endif  /* defined TEST_[= (. pname-up) =]_OPTS */[=

ENDDEF  =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DECLARE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE declare-option-callbacks      =][=

   #  For test builds, no need to call option procs  =][=

  IF (exist? "test_main")            =]
#if ! defined( TEST_[=(. pname-up)=]_OPTS )[=
  ENDIF

  =]
/*
 *  Procedures to call when option(s) are encountered
 */[=


  FOR flag                           =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

    CASE arg_type                    =][=
    =*  key                          =]
static tOptProc doOpt[=(. cap-name)  =];[=
    =*  bool                         =][=
    =*  num                          =][=
    *                                =][=
      IF (exist? "call_proc")        =]
extern tOptProc [=(get "call_proc")  =];[=

      ELIF (exist? "flag_code")      =]
static tOptProc doOpt[=(. cap-name)  =];[=

      ENDIF                          =][=
    ESAC                             =][=
  ENDFOR                             =][=
 
  IF (exist? "test_main")            =][=

     # "A test environment is to be generated" =]

#else /* *is*  defined( TEST_[=(. pname-up)=]_OPTS ) */
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */[=
    FOR flag            =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

      IF (exist? "call_proc") =]
#define [=(get "call_proc")   =] [=
          IF (~ (get "max") "1{0,1}")
                        =](tpOptProc)NULL[=
          ELSE          =]stackOptArg[=
          ENDIF         =][=

      ELIF (exist? "flag_code")  =]
#define doOpt[=(. cap-name)   =] [=
          IF (~ (get "max") "1{0,1}")
                        =](tpOptProc)NULL[=
          ELSE          =]stackOptArg[=
          ENDIF         =][=

      ELIF (=* (get "arg_type") "key")  =]
static tOptProc doOpt[=(. cap-name)  =];[=
      ENDIF             =][=

    ENDFOR flag         =]
#endif /* defined( TEST_[=(. pname-up)=]_OPTS ) */[=
  ENDIF (exist? "test_main") =]
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
DEF_PROC_2( static void doOpt[=(. cap-name) =],
            tOptions*, pOptions, tOptDesc*, pOptDesc )
{
[=

ENDDEF                          =][=

# # # # # # # # # # # # # # # # =][=

DEFINE define-option-callbacks  =][=

  FOR  flag  =][=

    (set! UP-name    (string-upcase! (get "name")))
    (set! cap-name   (string-capitalize UP-name))
    (set! low-name   (string-downcase UP-name))      =][=

    IF (exist? "flag_code") =][=

      IF (exist? "test_main") =]

#if ! defined( TEST_[= (. pname-up) =]_OPTS )[=

      ENDIF =][=

      invoke callback-proc-header  =][=
      flag_code =]
}[=

  IF (exist? "test_main") =]

#endif /* ! defined TEST_[= (. pname-up) =]_OPTS */[=

  ENDIF =][=


    ELIF (=* (get "arg_type") "key") =][=

      invoke callback-proc-header  =][=
      IF (not (exist? "arg_default"))
=]    tSCC zDef[2] = { 0x7F, 0 };
[=    ENDIF =][=

    IF (> (len "arg_optional") 0) =]
    te_[=(string-append Cap-prefix cap-name)=] def_val = [=
       IF (not (=* (get "arg_optional") low-name))
             =][=(. UP-name)=]_[=
       ENDIF =][=(string-upcase! (string->c-name! (get "arg_optional")))
             =];[=
    ENDIF
=]    tSCC* az_names[] = {[=
      IF (not (exist? "arg_default")) =] zDef,[=
      ENDIF  =]
[=(shellf "columns -I8 --spread=2 --sep=',' -f'\"%%s\"' <<_EOF_\n%s\n_EOF_\n"
          (join "\n" (stack "keyword")) )=]
    };
[=

    IF (> (len "arg_optional") 0) =]
    if (((tUL)pOptions > 0x0FUL) && (pOptDesc->pzLastArg == NULL))
        pOptDesc->pzLastArg = (char*)def_val;
    else[=
    ENDIF =]
    pOptDesc->pzLastArg =
        optionEnumerationVal( pOptions, pOptDesc, az_names, [=
        (+ (count "keyword") (if (exist? "arg_default") 0 1)) =] );
}[=
    ENDIF       =][=
  ENDFOR flag   =]
[=

ENDDEF

=]
