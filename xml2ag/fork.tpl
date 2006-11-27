[= AutoGen5 Template c=fork.c -*- Mode: C -*- =]
[= # Time-stamp:        "2006-11-26 16:51:58 bkorb" =][=

(define up-c-name  (lambda (ag-name)
  (string-upcase! (string->c-name! (get ag-name)))  ))

(dne " *  " "/*  ")=]
 *
 *  This module will fire up autogen and have it read definitions
 *  from its standard-in.
 */

static void
addArg( char const* pzArg, int ix )
{
    char** pArgv = xml2agOptions.origArgVect;
    if (ix >= xml2agOptions.origArgCt) {
        xml2agOptions.origArgCt += 5;
        pArgv = realloc( pArgv, sizeof( void* ) * (xml2agOptions.origArgCt+1) );
        if (pArgv == NULL) {
            fprintf( stderr, "No memory for %d args\n",
                     xml2agOptions.origArgCt );
            exit( EXIT_FAILURE );
        }
        xml2agOptions.origArgVect = pArgv;
    }
    pArgv[ ix ] = (void*)pzArg;
}


void
forkAutogen( char const* pzInput )
{
    tSCC zErr[] = "%s fs ERROR %d (%s) on %s\n";
    int fd[2];

    if (pipe( fd ) != 0) {
        fprintf( stderr, zErr, xml2agOptions.pzProgName,
                 errno, strerror( errno ), "pipe(2)" );
        exit( EXIT_FAILURE );
    }

    fflush( stdout );
    fflush( stdin  );

    switch (fork()) {
    case -1:
        fprintf( stderr, zErr, xml2agOptions.pzProgName,
                 errno, strerror( errno ), "fork(2)" );
        exit( EXIT_FAILURE );

    case 0:
        fclose( stdin );
        if (dup2( fd[0], STDIN_FILENO ) != STDIN_FILENO) {
            fprintf( stderr, zErr, xml2agOptions.pzProgName,
                     errno, strerror( errno ), "dup2(2) w/ STDIN_FILENO" );
            exit( EXIT_FAILURE );
        }
        close( fd[1] );
        break;

    default:
        errno = 0;
        outFp = fdopen( fd[1], "w" );
        if (outFp == NULL) {
            fprintf( stderr, zErr, xml2agOptions.pzProgName,
                     errno, strerror( errno ), "fdopen(2) w/ pipe[1]" );
            exit( EXIT_FAILURE );
        }
        close( fd[0] );
        return;
    }

    if (! HAVE_OPT( BASE_NAME )) {
        if (pzInput == NULL)
            pzInput = "stdin";
        else {
            char* pz = strrchr( pzInput, '.' );
            if (pz != NULL) {
                pzInput = pz = strdup( pzInput );
                pz = strrchr( pz, '.' );
                *pz = '\0';
            }
        }
        SET_OPT_BASE_NAME( pzInput );
    }

    {
        tSCC zAg[] = "autogen";
        char*  pzArg;
        int    ix    = 1;

        {
            char* pz = malloc( strlen( xml2agOptions.pzProgPath ) + 7 );
            char* p  = strrchr( xml2agOptions.pzProgPath, '/' );

            if (p == NULL) {
                strcpy( pz, zAg );
            } else {
                size_t len = (p - xml2agOptions.pzProgPath) + 1;
                memcpy( pz, xml2agOptions.pzProgPath, len );
                strcpy( pz + len, zAg );
            }

            addArg( pz, 0 );
        }[=

        FOR flag                   =][=
          IF (define opt-name (up-c-name "name"))
             (not (~~ opt-name "OVERRIDE_TPL|OUTPUT") ) =]

        if (HAVE_OPT( [=(. opt-name)=] )) {[=

          CASE arg-type            =][=

          ==*  key                 =]
            tSCC* kwlist[] = {
[=(shellf "${CLexe:-columns} -I16 -f'\"%%s\"' -S, --spread=2 <<_EOF_\n%s\n_EOF_"
   (join "\n" (stack "keyword"))  )=] };
            pzArg = malloc( [= (+ 4 (string-length (get "name")))
                        =] + strlen( kwlist[ OPT_VALUE_[=(. opt-name)=] ] ) );
            sprintf( pzArg, "--[=name=]=%s", kwlist[ OPT_VALUE_[=
                        (. opt-name)=] ] );
            addArg( pzArg, ix++ );[=

          ==*  num                 =]
            pzArg = malloc( (size_t)[= (+ 16 (string-length (get "name")))
                        =] );
            sprintf( pzArg, "--[=name=]=%d", (int)OPT_VALUE_[=(. opt-name)=] );
            addArg( pzArg, ix++ );[=

          ==*  bool                =]
            static char z[] = "--[=name=]=false";
            if (OPT_VALUE_[=(. opt-name)=])
                strcpy( z + [= (+ 3 (string-length (get "name"))) =], "true" );
            addArg( z, ix++ );[=

          ==*  str                 =][=
               IF (exist? "max")   =]
            int    optCt = STACKCT_OPT( [=(. opt-name)=] );
            char const**  ppOA  = STACKLST_OPT( [=(. opt-name)=] );
            do  {
                char const* pA = *(ppOA++);
                pzArg = malloc( [= (+ 4 (string-length (get "name")))
                        =] + strlen( pA ));
                sprintf( pzArg, "--[=name=]=%s", pA );
                addArg( pzArg, ix++ );
            } while (--optCt > 0);[=
               ELSE !exists-max    =]
            pzArg = malloc( [= (+ 4 (string-length (get "name")))
                        =] + strlen( OPT_ARG( [=(. opt-name)=] )));
            sprintf( pzArg, "--[=name=]=%s", OPT_ARG( [=(. opt-name)=] ));
            addArg( pzArg, ix++ );[=
               ENDIF exists-max    =][=

          ==   ""                  =]
            addArg( "--[=name=]", ix++ );[=

          ESAC arg-type            =]
        }[=
          ENDIF (not override)  =][=
        ENDFOR                     =]

        xml2agOptions.origArgVect[ ix ] = NULL;
        execvp( xml2agOptions.origArgVect[0], xml2agOptions.origArgVect );

        /*
         *  IF the first try fails, it may be because xml2ag and autogen have
         *  different paths.  Try again with just plain "autogen" and let
         *  the OS search "PATH" for the program.
         */
        execvp( zAg, xml2agOptions.origArgVect );
        fprintf( stderr, zErr, xml2agOptions.pzProgName,
                 errno, strerror( errno ), "execvp(2)" );
        exit( EXIT_FAILURE );
    }
}

/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autogen.c */
