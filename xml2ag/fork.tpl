[= AutoGen5 Template c=fork.c -*- Mode: C -*- =]
[=(dne " *  " "/*  ")=]
 *
 *  This module will fire up autogen and have it read definitions
 *  from its standard-in.
 */
#include "opts.h"
#  include <fcntl.h>

static void
addArg( char* pzArg, int ix )
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
    pArgv[ ix ] = pzArg;
}


void
forkAutogen( char* pzInput )
{
    FILE* fp;
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
            int err = errno;
            fprintf( stderr, "outFp is NULL, stdout is 0x%08p\n", stdout );
            fprintf( stderr, zErr, xml2agOptions.pzProgName,
                     err, strerror( err ), "fdopen(2) w/ pipe[1]" );
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
                pzInput = strdup( pzInput );
                pz = strrchr( pzInput, '.' );
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
                struct stat sbf;
                size_t len = (p - xml2agOptions.pzProgPath) + 1;
                memcpy( pz, xml2agOptions.pzProgPath, len );
                strcpy( pz + len, zAg );
            }

            addArg( pz, 0 );
        }[=

        FOR flag                   =][=
          IF (define opt-name (string-tr (get "name") "a-z^-" "A-Z__"))
             (not (~~ opt-name "OVERRIDE_TPL|OUTPUT") ) =]

        if (HAVE_OPT( [=(. opt-name)=] )) {[=

          CASE arg-type            =][=

          ==*  key                 =]
            tSCC* kwlist[] = {
[=(shellf "columns -I16 -f'\"%%s\"' -S, --spread=2 <<_EOF_\n%s\n_EOF_"
   (join "\n" (stack "keyword"))  )=] };
            pzArg = malloc( [= (+ 4 (string-length (get "name")))
                        =] + strlen( kwlist[ OPT_VALUE_[=(. opt-name)=] ] ) );
            sprintf( pzArg, "--[=name=]=%s", kwlist[ OPT_VALUE_[=(. opt-name)=] ] );
            addArg( pzArg, ix++ );[=

          ==*  num                 =]
            pzArg = malloc( [= (+ 16 (string-length (get "name")))
                        =] );
            sprintf( pzArg, "--[=name=]=%d", OPT_VALUE_[=(. opt-name)=] );
            addArg( pzArg, ix++ );[=

          ==*  bool                =]
            static char z[] = "--[=name=]=false";
            if (OPT_VALUE_[=(. opt-name)=])
                strcpy( z + [= (+ 3 (string-length (get "name"))) =], "true" );
            addArg( z, ix++ );[=

          ==*  str                 =][=
               IF (exist? "max")   =]
            int    optCt = STACKCT_OPT( [=(. opt-name)=] );
            char** ppOA  = STACKLST_OPT( [=(. opt-name)=] );
            do  {
                char* pA = *(ppOA++);
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
 * tab-width: 4
 * End:
 * end of autogen.c */
