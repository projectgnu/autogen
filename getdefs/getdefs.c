
#define DEFINE
#include "getdefs.h"

static const char zStartDef[] =
"%s = {\n    name = '%s';\n";

static const char zSeekErr[] =
"Error %d (%s) seeking to start of %sevt.num\n";

static const char zMallocErr[] =
"Error:  could not allocate %d bytes for %s\n";

static const char zAttribRe[] =
    "\n[^*]*\\*[ \t]*([a-z][a-z0-9_]*):";

static const char zNameTag[] =
    " = {\n    name = '";

char zRER[ 256 ];

char*   pzDefPat = (char*)NULL;
regex_t define_re;

char*  pzEvtNum = (char*)NULL;
char   zCurDir[ 65 ];
char   zBaseName[ 65 ];

FILE*  evtFp = (FILE*)NULL;
size_t evtSz = 0;

typedef int (compar_func)(const void *, const void *);
compar_func compar_text;

typedef char* tPz;
tPz*    papzBlocks = (tPz*)NULL;
size_t  blkUseCt   = 0;
size_t  blkAllocCt = 0;

FILE* startAutogen( void );
char* loadFile( char* pzFname );
void  processFile( char* pzFile );
void  sortEntries( void );
void  validateOptions( void );
void  printEntries( FILE* defFp );


    int
main( int    argc,
      char** argv )
{
    optionProcess( &getdefsOptions, argc, argv );
    validateOptions();

    {
        FILE* outFp;
        const char zAgDef[] = "autogen definitions %s;\n";

        outFp = startAutogen();

        fprintf( outFp, zAgDef, zBaseName );

        if (HAVE_OPT( INCLUDE )) {
            char* pz = loadFile( OPT_ARG( INCLUDE ));
            fputs( pz, outFp );
            fputc( '\n', outFp );
            free( (void*)pz );
        }

        if (HAVE_OPT( ASSIGN )) {
            int    ct  = STACKCT_OPT(  ASSIGN );
            char** ppz = STACKLST_OPT( ASSIGN );
            do  {
                fprintf( outFp, "%s;\n", *ppz++ );
            } while (--ct > 0);
        }

        {
            int    ct  = STACKCT_OPT(  INPUT );
            char** ppz = STACKLST_OPT( INPUT );
            do  {
                processFile( *ppz++ );
            } while (--ct > 0);
        }

        if (ENABLED_OPT( ORDERING ))
            qsort( (void*)papzBlocks, blkUseCt, sizeof( char* ),
                   &compar_text );

        printEntries( outFp );
        fclose( outFp );
    }

    return EXIT_SUCCESS;
}


    void
validateOptions( void )
{
    if (! HAVE_OPT( DEFS_TO_GET )) {
        pzDefPat = "/\\*=([a-z][a-z]*) ";

    } else {
        char*  pz  = OPT_ARG( DEFS_TO_GET );
        size_t len = strlen( pz ) + 16;
        if (len < 16)
            len = 16;
        pzDefPat = (char*)malloc( len );
        if (pzDefPat == (char*)NULL) {
            fprintf( stderr, zMallocErr, len, "definition pattern" );
            exit( EXIT_FAILURE );
        }
        sprintf( pzDefPat, "/\\*=(%s) ", pz );
    }

    {
        static const char zReErr[] =
            "Regex error %d (%s):  Cannot compile reg expr:\n\t%s\n";

        int rerr = regcomp( &define_re, pzDefPat, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &define_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, pzDefPat );
            exit( EXIT_FAILURE );
        }

        rerr = regcomp( &attrib_re, zAttribRe, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &attrib_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, zAttribRe );
            exit( EXIT_FAILURE );
        }
    }

    if (HAVE_OPT( BLOCK )) {
        int     ct  = STACKCT_OPT(  BLOCK );
        char**  ppz = STACKLST_OPT( BLOCK );

        do  {
            char* pz = strchr( *ppz++, '=' );
            if (pz == (char*)NULL) {
                fprintf( stderr, "ERROR:  block attr must have name list:\n"
                         "\t%s\n", ppz[-1] );
                USAGE( EXIT_FAILURE );
            }
            *pz = NUL;
        } while (--ct > 0);
    }
}



    int
compar_text( const void* p1, const void* p2 )
{
    char* pz1 = strstr( *(char**)p1, zNameTag );
    char* pe1;
    char* pz2 = strstr( *(char**)p2, zNameTag );
    char* pe2;
    int   res;

    static const char zBogus[] = "Bogus definition:\n%s\n";

    if (pz1 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p1 );
        exit( EXIT_FAILURE );
    }

    if (pz2 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p2 );
        exit( EXIT_FAILURE );
    }

    pe1 = strchr( pz1 + sizeof( zNameTag ), '\'' );

    if (pe1 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p1 );
        exit( EXIT_FAILURE );
    }

    pe2 = strchr( pz2 + sizeof( zNameTag ), '\'' );

    if (pe2 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p2 );
        exit( EXIT_FAILURE );
    }

    *pe1 = *pe2 = NUL;
    res = strcmp( pz1, pz2 );
    *pe1 = *pe2 = '\'';
    return res;
}


    int
eventNumber( char* pzName )
{
    char* pz;
    char* pzGrp;
    char  zName[ 80 ];
    int   num = 0;

    strcpy( zName, "#define " );
    pz = zName + sizeof( "#define " )-1;
    pzGrp = zCurDir;
    while (*pzGrp != NUL)
        *pz++ = toupper( *pzGrp++ );
    *pz++ = '_';
    while (*pzName != NUL)
        *pz++ = toupper( *pzName++ );
    strcpy( pz, "_EVTNO " );
    pzGrp = strstr( pzEvtNum, zName );
    if (pzGrp != (char*)NULL) {
        pzGrp += strlen( zName );
        while (isspace( *pzGrp )) pzGrp++;
        return atoi( pzGrp );
    }
    pzGrp = pzEvtNum + strlen( pzEvtNum );
    while ((pzGrp > pzEvtNum) && isspace( pzGrp[-1] )) pzGrp--;
    while ((pzGrp > pzEvtNum) && isdigit( pzGrp[-1] )) pzGrp--;
    if (isdigit( *pzGrp ))
        num = atoi( pzGrp )+1;
    pzGrp += strlen( pzGrp );
    sprintf( pzGrp, "%-40s %5d\n", zName, num );
    return num;
}


    int
putBlockAttr(  char** ppzAttr, char* pzAttrName, FILE* outFp )
{
    char* pzAttrDefs;

    fprintf( outFp, "    %s", pzAttrName );

    if (! HAVE_OPT( BLOCK ))
        return 0;

    {
        int     ct  = STACKCT_OPT(  BLOCK );
        char**  ppz = STACKLST_OPT( BLOCK );

        for (;;) {
            if (streqvcmp( *ppz++, pzAttrName ) == 0)
                break;
            if (--ct <= 0)
                return 0;
        }
        pzAttrDefs = ppz[-1];
        pzAttrDefs += strlen( pzAttrDefs ) + 1;
    }

    fputs( " = {\n", outFp );
    {
        char* pzAttr = *ppzAttr + 1;
        char* pzDta  = (char*)malloc( strlen( pzAttr ));
        char* pzScn  = pzDta;
        pzAttr += strspn( pzAttr, " \t" );
        if (*pzAttr != '\n') {
            do  {
                *pzScn++ = *pzAttr++;
            } while ((*pzAttr != NUL) && (*pzAttr != '\n'));
            *pzScn++ = '\n';
        }
        for (;;) {
            char* pz = strpbrk( pzAttr+1, "*\n" );
            if (pz == (char*)NULL)
                break;
            pzAttr = pz;
            if (*pz == '\n') {
                *pzScn++ = '\n';
                pzAttr = pz;
                continue;
            }
            if (isalpha( pz[1] )) {
                pzAttr = pz-1;
                *pzAttr = '\n';
                break;
            }
            pz++;
            if (*pz == '.')
                 pz++;
            else pz += strspn( pz, " \t" );
            for (;;) {
                switch (*pzScn++ = *pz++) {
                case '\n':
                    pzAttr = pz;
                    goto endInnerCopy;

                case '\0':
                    pzAttr = pz-1;
                    goto endCopy;

                default:
                    ;
                }
            } endInnerCopy: ;
        }

    endCopy:
        *ppzAttr = pzAttr;
        while (isspace( pzScn[-1] )) pzScn--;
        *pzScn   = NUL;
        pzScn    = pzDta;
        while (isspace( *pzScn )) pzScn++;

        while (*pzScn != NUL) {
            fputs( "        ", outFp );
            while (isalnum( *pzAttrDefs ) || (*pzAttrDefs == '_'))
                fputc( *pzAttrDefs++, outFp );
            fputs( " = '", outFp );
            while (isspace( *pzAttrDefs )) pzAttrDefs++;
            if (*pzAttrDefs == ',') {
                pzAttrDefs++;
                while (isspace( *pzAttrDefs )) pzAttrDefs++;
            }
            for (;;) {
                switch (*pzScn) {
                case '\'':
                    fputc( '\\', outFp );
                default:
                    fputc( *pzScn++, outFp );
                    break;
                case ',':
                    pzScn++;
                case NUL:
                    fputs( "';\n", outFp );
                    while (isspace( *pzScn )) pzScn++;
                    goto dataDone;
                }
            } dataDone:;
        }

        free( (void*)pzDta );
    }
    fputs( "    };\n", outFp );
    return 1;
}


typedef enum {
    DST_START,
    DST_NAME_LINE,
    DST_NAME,
    DST_TEXT,
    DST_STRING
} teDefState;



    void
compressDef( char* pz )
{
    char* pzStrt = pz;
    char* pzDest = pz;
    char* pzSrc  = pz+1;

    for (;;) {
        int   nl =  0;

        /*
         *  Skip over leading space
         */
        while (isspace( *pzSrc )) {
            if (*pzSrc == '\n')
                nl++;
            pzSrc++;
        }

        /*
         *  IF no new-lines were found, then we found text start
         */
        if (nl == 0)
            break;

        /*
         *  Skip forward to the next asterisk.
         *  Then, skip over leading space.
         */
        while (*++pzSrc != '*') {
            if (*pzSrc == NUL) {
                *pzStrt = NUL;
                return;
            }
        }
    }

    for (;;) {
        for (;;) {
	    /*
	     *  Move the source to destination until we find
	     *  either a new-line or a NUL.
	     */
            switch (*pzDest++ = *pzSrc++) {
            case '\n':
                goto lineDone;
            case NUL:
                pzDest--;
                goto compressDone;
            default:
            }
        } lineDone:;

	/*
	 *  We found a new-line.  Skip forward to an asterisk.
	 */
        while (*++pzSrc != '*') {
            if (*pzSrc == NUL) {
                goto compressDone;
            }
        }
        while (isspace( *++pzSrc ))  ;
    } compressDone:;

    while ((pzDest > pzStrt) && isspace( pzDest[-1] )) pzDest--;
    *pzDest = NUL;
}



    void
buildDefinition( char* pzDef, char* pzFile, int line, char* pzOut )
{
    static const char zFieldMark[] =
        "error field name does not end with ':' in file %s line %d\n";
    static const char zStrCtx[] =
        "error String value found out of context in file %s line %d\n";
    static const char zConfused[] =
        "error getdefs confusion in file %s line %d\n";
    static const char zNoData[] =
        "error no data for definition in file %s line %d\n";

    teDefState state = DST_START;
    char* pzNextDef;

    while (isalnum( *pzDef ))
        *pzOut++ = *pzDef++;

    while (isspace( *pzDef )) pzDef++;

    {
        char* pzName = pzDef;
        while (! isspace( *pzDef )) {
            if (*pzDef == NUL)
                return;
            pzDef++;
        }
        *pzDef = NUL;

        pzOut += sprintf( pzOut, "%s%s';\n", zNameTag, pzName );
    }

    if (HAVE_OPT( BLOCK )) {
        int    ct  = STACKCT_OPT(  BLOCK );
        char** ppz = STACKLST_OPT( BLOCK );
        do  {
            pzOut += sprintf( "    %s;\n", *ppz++ );
        } while (--ct > 0);
    }

    *pzDef = '\n';
    for (;; pzDef = pzNextDef) {
        int        ct;
        regmatch_t match[2];

        ct = regexec( &attrib_re, pzDef, COUNT( match ), match, 0 );
        if (ct != 0) {
            if (ct != 1) {
                regerror( ct, &attrib_re, zRER, sizeof( zRER ));
                fprintf( stderr, "Error %d (%s) finding `%s' in\n%s\n\n",
                         ct, zRER, zAttribRe, pzDef );
            }
            break;
        }
        pzDef[ match[0].rm_so ] = NUL;
        pzNextDef = pzDef + match[1].rm_so;
        if (state == DST_START) {
            state = DST_NAME_LINE;
            continue;
        }
        *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' ';
        while (*pzDef != ':') *pzOut++ = *pzDef++;
        compressDef( pzDef );

        switch (*pzDef) {
        case NUL:
            *pzOut++ = ';'; *pzOut++ = '\n';
            break;

        case '"':
        case '\'':
            pzOut += sprintf( pzOut, " = %s;\n", pzDef );
            break;

        case '\n':
            *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = '\n';
            pzDef++;

            switch (*pzDef) {
            case '"':
            case '\'':
                pzOut += sprintf( pzOut, " = %s;\n", pzDef );
                break;

            default:
                pzOut += sprintf( pzOut, " = '%s';\n", pzDef );
                break;
            }

        default:
            pzOut += sprintf( pzOut, " = '%s';\n", pzDef );
            break;
        }
    }

    switch (state) {
        case DST_START:
            *pzOut++ = '\n'; *pzOut++ = '#';
            pzOut += sprintf( pzOut,  zNoData, pzFile, line );
            fprintf( stderr, zNoData, pzFile, line );
            return;

        case DST_TEXT:
            *pzOut++ = '\'';

        case DST_STRING:
        case DST_NAME_LINE:
        case DST_NAME:
            *pzOut++ = ';'; *pzOut++ = '\n';
    }

    *pzOut++ = '}'; *pzOut++ = ';'; *pzOut++ = '\n'; *pzOut++ = NUL;
}



    void
processFile( char* pzFile )
{
    char* pzText = loadFile( pzFile ); /* full text */
    char* pzScan;  /* Scanning Pointer  */
    char* pzDef;   /* Def block start   */
    char* pzNext;  /* start next search */
    char* pzDta;   /* data value        */
    int   lineNo = 1;
    char* pzOut;

    regmatch_t  matches[MAX_SUBMATCH+1];
    pzNext = pzText;

    while ( pzScan = pzNext,
            regexec( &define_re, pzScan, COUNT(matches), matches, 0 ) == 0) {

        static const char zNoEnd[] =
            "Error:  definition in %s at line %d has no end\n";
        static const char zNoSubexp[] =
            "Warning: subexpr not found on line %d in %s:\n\t%s\n";

        /*
         *  Make sure there is a subexpression match!!
         */
        if (matches[1].rm_so == -1) {
            char* pz;
            char  ch;

            pzDef = pzScan + matches[0].rm_so;
            if (strlen( pzDef ) > 30) {
                pz  = pzDef + 30;
                ch  = *pz;
                *pz = NUL;
            } else
                pz = (char*)NULL;

            fprintf( stderr, zNoSubexp, pzFile, lineNo, pzDef );
            if (pz != (char*)NULL)
                *pz = ch;
            continue;
        }

        pzDef = pzScan + matches[1].rm_so;
        pzNext = strstr( pzDef, "=*/" );
        if (pzNext == (char*)NULL) {
            fprintf( stderr, zNoEnd, pzFile, lineNo );
            exit( EXIT_FAILURE );
        }
        *pzNext = NUL;
        pzNext += 3;
        /*
         *  Count the number of lines skipped to the start of the def.
         */
        while (pzScan < pzDef) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan == (char*)NULL)
                break;
            lineNo++;
            pzScan++;
        }

        {
            tSCC zLineId[] = "\n#line %d \"%s\"\n";
            pzOut = pzDta =
                (char*)malloc( 2 * strlen( pzDef ) + sizeof( zLineId ) + 4096);
            pzOut += sprintf( pzDta, zLineId, lineNo, pzFile );
        }

        /*
         *  Count the number of lines in the definition itself.
         *  It will find and stop on the "=* /\n" line.
         */
        pzScan = pzDef;
        for (;;) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan++ == (char*)NULL)
                break;
            lineNo++;
        }

        /*
         *  OK.  We are done figuring out where the boundaries of the
         *  definition are and where we will resume our processing.
         */
        buildDefinition( pzDef, pzFile, lineNo, pzOut );
        pzDta = (char*)realloc( (void*)pzDta, strlen( pzDta ) + 1 );

        if (++blkUseCt > blkAllocCt) {
            blkAllocCt += 32;
            papzBlocks = (char**)realloc( (void*)papzBlocks,
                                          blkAllocCt * sizeof( char* ));
            if (papzBlocks == (char**)NULL) {
                fprintf( stderr, "Realloc error for %d pointers\n",
                         blkAllocCt );
                exit( EXIT_FAILURE );
            }
        }
        papzBlocks[ blkUseCt-1 ] = pzDta;
    }

    if (lineNo == 1) {
        fprintf( stderr, "Warning:  no copies of pattern `%s' were found in "
                 "%s\n", pzDefPat, pzFile );
    }

    free( (void*)pzText );
}


    void
printEntries( FILE* fp )
{
    int     ct  = blkUseCt;
    char**  ppz = papzBlocks;

    for (;;) {
        char* pz = *(ppz++);
        if (--ct < 0)
            break;
        fputs( pz, fp );
        free( (void*)pz );
        if (ct > 0)
            fputs( "\n\n", fp );
    }
    free( (void*)papzBlocks );
}


    FILE*
startAutogen( void )
{
    char* pz;
    FILE* agFp;

    char* args[ 8 ];
    char** pparg = args+1;
    char*  pzAutogen = "autogen";

    char zTempl[ MAXPATHLEN ];
    char zSrch[  MAXPATHLEN ];
    char zBase[  MAXPATHLEN ];

    if (HAVE_OPT( AUTOGEN ))
        switch (WHICH_IDX_AUTOGEN) {
        case INDEX_OPT_OUTPUT:
            if (strcmp( OPT_ARG( OUTPUT ), "-"))
                return stdout;
            return fopen( OPT_ARG( OUTPUT ), "r" );

        case INDEX_OPT_AUTOGEN:
            if (! ENABLED_OPT( AUTOGEN ))
                return stdout;
    
            if (  ( OPT_ARG( AUTOGEN ) != (char*)NULL)
               && (*OPT_ARG( AUTOGEN ) != NUL ))
                pzAutogen = OPT_ARG( AUTOGEN );

            break;

        default:
            break;
        }

    if (HAVE_OPT( BASE_NAME ))
        sprintf( zBase, "-b%s", OPT_ARG( BASE_NAME ));

    else if (HAVE_OPT( DEFS_TO_GET )) {
        char* pzS = OPT_ARG( DEFS_TO_GET );
        strcpy( zBase, "-b" );
        pz = zBase + 2;
        while (isalnum( *pzS ) || (*pzS == '_'))
            *pz++ = *pzS++;
        *pz = NUL;
    }

    if (zBase[2] == NUL) {
        char zPath[  MAXPATHLEN ];
        if (getcwd( zPath, sizeof( zPath )) == (char*)NULL) {
            fprintf( stderr, "Error %d (%s) on getcwd\n", errno,
                     strerror( errno ));
            exit( EXIT_FAILURE );
        }

        pz = strrchr( zPath, '/' );
        if (pz == (char*)NULL)
             pz = zPath;
        else pz++;
        strcpy( zBase+2, pz );
    }

    if (! ENABLED_OPT( AUTOGEN )) {
        strcpy( zBaseName, zBase+2 );
        return stdout;
    }

    args[0] = pzAutogen;

    if (HAVE_OPT( TEMPL_DIRS )) {
        sprintf( zSrch, "-L%s", OPT_ARG( TEMPL_DIRS ));
        *pparg++ = zSrch;
    }

    *pparg++ = zBase;
    strcpy( zBaseName, zBase+2 );

    if (HAVE_OPT( TEMPLATE )) {
        sprintf( zTempl, "-T%s", OPT_ARG( TEMPLATE ));
        *pparg++ = zTempl;
    }

    *pparg++ = "--";
    *pparg++ = "-";
    *pparg++ = (char*)NULL;

    {
        int  pfd[2];

        if (pipe( pfd ) != 0) {
            fprintf( stderr, "Error %d (%s) creating pipe\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );
        }

        switch (fork()) {
        case 0:
            close( pfd[1] );
            if (dup2( pfd[0], STDIN_FILENO ) != 0) {
                fprintf( stderr, "Error %d (%s) dup pipe[0]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }

            execvp( args[0], args );
            fprintf( stderr, "Error %d (%s) exec of %s %s %s %s %s\n",
                     errno, strerror( errno ),
                     args[0], args[1], args[2], args[3], args[4] );
            exit( EXIT_FAILURE );

        case -1:
            fprintf( stderr, "Error %d (%s) on fork()\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );

        default:
            agFp = fdopen( pfd[1], "w" );
            if (agFp == (FILE*)NULL) {
                fprintf( stderr, "Error %d (%s) fdopening pipe[1]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }
            close( pfd[0] );
        }
    }

    return agFp;
}


    char*
loadFile( char* pzFname )
{
    FILE* fp = fopen( pzFname, "r" );
    int   res;
    struct stat stb;
    char*  pzText;

    if (fp == (FILE*)NULL) {
        fprintf( stderr, "Error %d (%s) read opening %s\n",
                 errno, strerror( errno ), pzFname );
        exit( EXIT_FAILURE );
    }
    res = fstat( fileno( fp ), &stb );
    if (res != 0) {
        fprintf( stderr, "error %d (%s) stat-ing %s\n",
                 errno, strerror( errno ), pzFname );
        exit( EXIT_FAILURE );
    }
    pzText = (char*)malloc( stb.st_size + 1 );
    if (pzText == (char*)NULL) {
        fprintf( stderr, "Error: could not allocate %d bytes\n",
                 stb.st_size + 1 );
        exit( EXIT_FAILURE );
    }
    if (fread( (void*)pzText, 1, stb.st_size, fp )
        != stb.st_size) {
        fprintf( stderr, "Could not read all %d bytes from %s\n",
                 stb.st_size, pzFname );
        exit( EXIT_FAILURE );
    }
    pzText[ stb.st_size ] = NUL;
    fclose( fp );
    return pzText;
}
