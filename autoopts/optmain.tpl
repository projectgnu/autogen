[= AutoGen5 Template -*- Mode: C -*- =]
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
    /* export_options_to_guile( &[=(. pname)=]Options ); */
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

DEFINE declare-option-callbacks  =][=

   #  For test builds, no need to call option procs  =][=

  IF (exist? "test_main") =]
#if ! defined( TEST_[=(. pname-up)=]_OPTS )[=
  ENDIF

  =]
/*
 *  Procedures to call when option(s) are encountered
 */[=


  FOR flag =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

    IF (exist? "call_proc") =]
extern tOptProc [=(get "call_proc")=];[=

    ELIF (exist? "flag_code") =]
static tOptProc doOpt[=(. cap-name)=];[=

    ENDIF =][=
  ENDFOR flag   =][=
 
  IF (exist? "test_main") =][=

     # "A test environment is to be generated" =]

#else /* *is*  defined( TEST_[=(. pname-up)=]_OPTS ) */
/*
 *  Under test, omit argument processing, or call stackOptArg,
 *  if multiple copies are allowed.
 */[=
    FOR flag =][=
    (set! cap-name (string-capitalize! (get "name"))) =][=

      IF (exist? "call_proc") =]
#define [=(get "call_proc")   =] [=
          IF (~ (get "max") "1{0,1}")
               =](tpOptProc)NULL[=
          ELSE =]stackOptArg[=
          ENDIF=][=

      ELIF (exist? "flag_code")  =]
#define doOpt[=(. cap-name)   =] [=
          IF (~ (get "max") "1{0,1}")
               =](tpOptProc)NULL[=
          ELSE =]stackOptArg[=
          ENDIF=][=

      ENDIF=][=

    ENDFOR flag=]
#endif /* defined( TEST_[=(. pname-up)=]_OPTS ) */[=
  ENDIF (exist? "test_main") =]
[=

ENDDEF

=][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

   DEFINE OPTION CALLBACKS

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE define-option-callbacks  =][=

  IF (exist? "test_main") =]

#if ! defined( TEST_[= (. pname-up) =]_OPTS )[=

  ENDIF =][=

  FOR flag =][=

    IF (exist? "flag_code") =]

/* * * * * * *
 *
 *   For the "[=(string-capitalize! (get "name"))=] Option".
 */
    static void
doOpt[=(string-capitalize (get "name"))
     =]( tOptions* pOptions, tOptDesc* pOptDesc )
{
[=flag_code=]
}[=
    ENDIF "flag_code _exist" =][=
  ENDFOR flag =][=

  IF (exist? "test_main") =]

#endif /* ! defined TEST_[= (. pname-up) =]_OPTS */[=

  ENDIF =]
[=

ENDDEF

=]
