
/*
 *  allbut
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef NUL
#define NUL '\0'
#endif

static char zUsage[] =
"USAGE:  %s <selection-pattern> '-' <exclusion-pattern>\n\n"
    "Echoes all of its arguments that appear before a hyphen argument,\n"
    "except those arguments that match arguments that appear after the\n"
    "hyphen.  Example:\n\n"

    "    allbut a b c - b \n\n"

    "will echo:\n\n"

    "    a c \n\n"

    "This is generally useful for such things as listing all the files\n"
    "in a directory that match one pattern but do not match another:\n\n"

    "    allbut x* - x*.c \n\n"

    "will list all files starting with 'x', except those ending in '.c'.\n"
    "The output will be sorted alphabetically and separated by the first\n"
    "character in the IFS environment variable, or by a space (' ').\n";

typedef char* tpChar;

int
sortProc( const void* ppVal1, const void* ppVal2 )
{ return strcmp( *(char**)ppVal1, *(char**)ppVal2 ); }

int
main( int argc, char** argv )
{
    int     argCt   = 0;
    char**  argList = (char**)malloc( sizeof( tpChar ) * (--argc) );
    char*   pArg    = getenv( "IFS" ); 
    char    sepChar;

    if (  (argc == 0)
       || (strcmp( argv[1], "-?") == 0 )
       || (strncmp(argv[1], "--h", 3) == 0 )  )  {
        fwrite(zUsage, sizeof(zUsage) - 1, 1, stderr);
        exit( EXIT_FAILURE );
    }

    if ((pArg == (char*)NULL) || (*pArg == NUL))
         sepChar = ' ';
    else sepChar = *pArg;

    /*
     *  First, stash arguments into the output list until we either
     *  run out of arguments or find the hyphen.  "argc" has been
     *  decremented and contains the count of arguments, exclusive of
     *  the program name ("argv[0]").
     */
    while (argCt < argc) {
        pArg = *(++argv);

        if ((*pArg == '-') && (pArg[1] == NUL)) {
            argc--;     /* hyphen found, start decrementing argc,
                         * instead of incrementing argCt
                         */
            break;
        }

        argList[ argCt++ ] = pArg;
    }

    /*
     *  Now, start removing entries until argc is decremented to zero
     */
    argc -= argCt;

    while (argc-- > 0) {
        char*   pArg   = *(++argv);
        int     ct     = argCt;
        char**  argPtr = argList;

        /*
         *  Scan what is left of the output list looking for a copy
         *  of the current argument.  Drop it if we find it.
         */
        while (ct-- > 0) {
            if (strcmp( *(argPtr++), pArg ) == 0) {
                argPtr[-1] = argList[ --argCt ];
                break;
            }
        }
    }

    /*
     *  IF we have any arguments left, then we do something
     */
    if (argCt > 0) {
        /*
         *  IF there are more than one left, then sort them
         *  We use the library procedure 'strcmp(3)' for the compare.
         */
        if (argCt > 1)
            qsort( (void*)argList, (size_t)argCt, sizeof( tpChar ),
                   sortProc );

        /*
         *  For each argument, print it.  We do the exit test in the
         *  middle of the loop, at the time we need to decide whether
         *  to emit a separator character or not.
         */
        for (;;) {
            /*
             *  We know we have one argument, so output it before
             *  testing for being done
             */
            fputs( *(argList++), stdout );

            /*
             *  If we are to continue,
             *  then we need a separator character, too.
             */
            if (--argCt <= 0)
                break;
            putchar( sepChar );
        }
    }

    putchar( '\n' );
    return EXIT_SUCCESS;
}
