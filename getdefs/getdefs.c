
#define DEFINE
#include "getdefs.h"

static const char zStartDef[] =
"%s = {\n    name = '%s';\n";

static const char zSeekErr[] =
"Error %d (%s) seeking to start of %sevt.num\n";

static const char zMallocErr[] =
"Error:  could not allocate %d bytes for %s\n";

static const char zAttribRe[] =
    "\n[^*\n]*\\*[ \t]*([a-z][a-z0-9_]*):";

static const char zNameTag[] =
    " = {\n    name = '";

#define MARK_CHAR ':'

char zRER[ 256 ];

#ifndef STR
#  define __STR(s)  #s
#  define STR(s)    __STR(s)
#endif

char*   pzDefPat = (char*)NULL;
regex_t define_re;

char*  pzEvtNum = (char*)NULL;
char   zCurDir[ 65 ];
char   zTemplName[ 65 ];

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

        fprintf( outFp, zAgDef, zTemplName );

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
        tSCC    zNoList[] = "ERROR:  block attr must have name list:\n"
                            "\t%s\n";
        int     ct  = STACKCT_OPT(  BLOCK );
        char**  ppz = STACKLST_OPT( BLOCK );
        char*   pz;
        char*   p;

        /*
         *  FOR each BLOCK argument,
         *  DO  condense each name list to be a list of names
         *      separated by a single space and NUL terminated.
         */
        do  {
            /*
             *  Make sure we find the '=' separator
             */
            pz = strchr( *ppz++, '=' );
            if (pz == (char*)NULL) {
                fprintf( stderr, zNoList, ppz[-1] );
                USAGE( EXIT_FAILURE );
            }

            /*
             *  NUL the equal char
             */
            *pz++ = NUL;
            p = pz;

            /*
             *  Make sure at least one attribute name is defined
             */
            while (isspace( *pz )) pz++;
            if (*pz == NUL) {
                fprintf( stderr, zNoList, ppz[-1] );
                USAGE( EXIT_FAILURE );
            }

            for (;;) {
                /*
                 *  Attribute names must start with an alpha
                 */
                if (! isalpha( *pz )) {
                    fprintf( stderr, "ERROR:  attribute names must start "
                             "with an alphabetic character:\n\t%s\n",
                             ppz[-1] );
                    USAGE( EXIT_FAILURE );
                }

                /*
                 *  Copy the name.  (maybe.  "p" and "pz" may be equal)
                 */
                while (isalnum( *pz ) || (*pz == '_'))
                    *p++ = *pz++;

                /*
                 *  Skip over one comma (optional) and any white space
                 */
                while (isspace( *pz )) pz++;
                if (*pz == ',')
                    pz++;

                while (isspace( *pz )) pz++;
                if (*pz == NUL)
                    break;
                /*
                 *  The final string contains only one space
                 */
                *p++ = ' ';
            }

            *p = NUL;
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





    void
compressDef( char* pz )
{
    char* pzStrt = pz;
    char* pzDest = pz;
    char* pzSrc  = pz+1;
    int   nlCt;

    for (;;) {
        nlCt =  0;

        /*
         *  Skip over leading space
         */
        while (isspace( *pzSrc )) {
            if (*pzSrc == '\n')
                nlCt++;
            pzSrc++;
        }

        /*
         *  IF no new-lines were found, then we found text start
         */
        if (nlCt == 0)
            break;

        /*
         *  Skip over the first asterisk we find
         *  Then, skip over leading space.
         */
        pzSrc = strchr( pzSrc, '*' );
        if (pzSrc == (char*)NULL) {
            *pzStrt = NUL;
            return;
        }

        /*
         *  Skip over sequential asterisks
         */
        while (*pzSrc == '*') pzSrc++;
    }

    for (;;) {
        for (;;) {
            /*
             *  Move the source to destination until we find
             *  either a new-line or a NUL.
             */
            switch (*pzDest++ = *pzSrc++) {
            case '\n':
                if (*pzSrc != NUL)
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
    foundNewline:
        while (*pzSrc != '*') {
            if (*pzSrc == NUL)
                goto compressDone;
            if (*pzSrc == '\n')
                nlCt++;
            pzSrc++;
        }
        if (nlCt > 0) {
            *pzDest++ = '\n';
            nlCt =  0;
        }
        while (*pzSrc == '*')     pzSrc++;
        while (isspace( *pzSrc )) {
            if (*pzSrc == '\n')
                goto foundNewline;
            pzSrc++;
        }
    } compressDone:;

    while ((pzDest > pzStrt) && isspace( pzDest[-1] )) pzDest--;
    *pzDest = NUL;
}


    char*
emitQuote( char** ppzText, char* pzOut )
{
    char*  pzText = *ppzText;
    char   svch   = (*pzOut++ = *pzText++);

    for (;;) {
        switch (*pzOut++ = *pzText++) {

        case '\\':
            if ((*pzOut++ = *pzText++) != NUL)
                break;

        case NUL:
            pzText--;
            pzOut[-1] = svch;
            svch = NUL;
            /* FALLTHROUGH */

        case '"':
        case '\'':
            if (pzOut[-1] == svch)
                goto quoteDone;

            break;
        }
    }

quoteDone:
    *ppzText = pzText;
    *pzOut++ = ';';
    return pzOut;
}



    char*
emitString( char** ppzText, char* pzOut )
{
    char*  pzText  = *ppzText;
    char*  pcComma;
    char*  pcEnd;

    while (isspace( *pzText )) pzText++;

    if ((*pzText == '"') || (*pzText == '\'')) {
        *ppzText = pzText;
        return emitQuote( ppzText, pzOut );
    }

    pcComma = strchr( pzText, ',' );
    if (pcComma == (char*)NULL) {
        pcEnd = pzText + strlen( pzText );
        pcComma = pcEnd-1;
    } else {
        pcEnd = pcComma;
    }

    while ((pcEnd > pzText) && isspace( pcEnd[-1] )) pcEnd--;
    *pzOut++ = '\'';
    {
        char ch = *pcEnd;
        *pcEnd = NUL;
        for (;;) {
            char ch = *pzText++;
            switch (ch) {
            case '\'':
                *pzOut++ = '\\';
            default:
                *pzOut++ = ch;
                break;
            case NUL:
                goto copyDone;
            }
        } copyDone: ;

        pzText = pcComma+1;
        *pcEnd = ch;
    }

    *pzOut++ = '\''; *pzOut++ = ';';
    *ppzText = pzText;
    return pzOut;
}



    char*
emitSubgroup( char* pzDefList, char* pzText, char* pzOut )
{
    tSCC  zStart[] = " = {\n        ";
    tSCC  zEnd[]   = "\n    };\n";
    int   newlineDone = 1;

    /*
     *  Advance past subgroup name to the entry name list
     */
    pzDefList += strlen( pzDefList ) + 1;
    strcpy( pzOut, zStart );
    pzOut += sizeof( zStart ) - 1;

    /*
     *  Loop for as long as we have text entries and subgroup
     *  attribute names, ...
     */
    do  {
        while (isspace( *pzText )) pzText++;
        if (*pzText == NUL)
            break;

        /*
         *  IF the text is just a comma, then we skip the entry
         */
        if (*pzText == ',') {
            pzText++;
            while ((! isspace( *pzDefList )) && (*pzDefList != NUL))
                pzDefList++;

        } else {
            if (! newlineDone) {
                strcpy( pzOut, zStart + 4 );
                pzOut += sizeof( zStart ) - 5;
            }

            /*
             *  Copy out the attribute name
             */
            for (;;) {
                *pzOut++ = *pzDefList++;
                if (*pzDefList == ' ') {
                    pzDefList++;
                    break;
                }
                if (*pzDefList == NUL)
                    break;
            }

            /*
             *  Copy out the assignment operator and emit the string
             */
            *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = ' ';
            pzOut = emitString( &pzText, pzOut );
            newlineDone = 0;
        }
    } while (isalpha( *pzDefList ));
    strcpy( pzOut, zEnd );
    return pzOut + sizeof( zEnd ) - 1;
}


    char*
emitDefinition( char* pzDef, char* pzOut )
{
    char   char_after_equal;
    char   zEntryName[ 256 ];

    /*
     *  Indent attribute definitions four spaces
     */
    *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' ';

    if (! HAVE_OPT( BLOCK )) {
        while (*pzDef != MARK_CHAR)  *pzOut++ = *pzDef++;
        compressDef( pzDef );

    } else {
        int    ct  = STACKCT_OPT(  BLOCK );
        char** ppz = STACKLST_OPT( BLOCK );
        char*  p   = zEntryName;

        while (*pzDef != MARK_CHAR)
            *p++ = *pzOut++ = *pzDef++;

        *p = NUL;
        compressDef( pzDef );

        do  {
            p = *ppz++;
            if (strcmp( p, zEntryName ) == 0) {
                pzOut = emitSubgroup( p, pzDef, pzOut );
                return pzOut;
            }
        } while (--ct > 0);
    }

    if (*pzDef == '\n') {
        char_after_equal = '\n';
        while (isspace( *pzDef )) pzDef++;
    } else {
        char_after_equal = ' ';
    }

    switch (*pzDef) {
    case NUL:
        *pzOut++ = ';'; *pzOut++ = '\n';
        break;

    case '"':
    case '\'':
    case '{':
        /*
         *  Quoted entries or subgroups do their own stringification
         */
        pzOut += sprintf( pzOut, " =%c%s;\n", char_after_equal, pzDef );
        break;

    default:
        *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = char_after_equal;
        *pzOut++ = '\'';

        for (;;) {
            switch (*pzOut++ = *pzDef++) {
            case '\\':
            case '\'':
                pzOut[-1] = '\\';
                *pzOut++  = '\'';
                break;

            case NUL:
                goto unquotedDone;
            }
        } unquotedDone:;
        pzOut[-1] = '\''; *pzOut++ = ';'; *pzOut++ = '\n';
        break;
    }
    return pzOut;
}


    void
buildDefinition(
    char*    pzDef,
    char*    pzFile,
    int      line,
    char*    pzOut )
{
    static const char zFieldMark[] =
        "error field name does not end with " STR( MARK_CHAR )
        " in file %s line %d\n";
    static const char zStrCtx[] =
        "error String value found out of context in file %s line %d\n";
    static const char zConfused[] =
        "error getdefs confusion in file %s line %d\n";
    static const char zNoData[] =
        "error no data for definition in file %s line %d\n";

    char* pzNextDef = (char*)NULL;

    /*
     *  Copy out the name of the entry type
     */
    while (isalnum( *pzDef ))
        *pzOut++ = *pzDef++;

    while (isspace( *pzDef )) pzDef++;

    /*
     *  Now find the name of this particular copy of the entry type
     */
    {
        char* pzName = pzDef;
        while (! isspace( *pzDef )) {
            if (*pzDef == NUL)
                return;
            pzDef++;
        }
        *pzDef = NUL;

        /*
         *  We insert the name with a consistent name string
         *  that we use to locate the sort key later.
         */
        pzOut += sprintf( pzOut, "%s%s';\n", zNameTag, pzName );
    }

    *pzDef = '\n';

    /*
     *  FOR each attribute for this entry, ...
     */
    for (;;) {
        int        re_res;
        regmatch_t match[2];

        /*
         *  Find the next attribute regular expression
         */
        re_res = regexec( &attrib_re, pzDef, COUNT( match ), match, 0 );
        switch (re_res) {
        case 0:
            /*
             *  NUL-terminate the current attribute.
             *  Set the "next" pointer to the start of the next attribute name.
             */
            pzDef[ match[0].rm_so ] = NUL;
            if (pzNextDef != (char*)NULL)
                pzOut = emitDefinition( pzNextDef, pzOut );
            pzNextDef = pzDef = pzDef + match[1].rm_so;
            break;

        case 1:
            /*
             *  No more attributes.
             */
            if (pzNextDef == (char*)NULL) {
                *pzOut++ = '\n'; *pzOut++ = '#';
                sprintf( pzOut,  zNoData, pzFile, line );
                fputs( pzOut, stderr );
                pzOut += strlen( pzOut );
                return;
            }

            pzOut = emitDefinition( pzNextDef, pzOut );
            goto eachAttrDone;
            break;

        default:
        {
            tSCC zErr[] = "error %d (%s) finding `%s' in\n%s\n\n";
            regerror( re_res, &attrib_re, zRER, sizeof( zRER ));
            *pzOut++ = '\n';
            *pzOut++ = '#';
            sprintf( pzOut, zErr, re_res, zRER, zAttribRe, pzDef );
            fputs( pzOut, stderr );
            return;
        }
        }
    } eachAttrDone:;

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
        for (;;) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan++ == (char*)NULL)
                break;
            if (pzScan >= pzDef)
                break;
            lineNo++;
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

    if (lineNo == 1)
        fprintf( stderr, "Warning:  no copies of pattern `%s' were found in "
                 "%s\n", pzDefPat, pzFile );

    free( (void*)pzText );
}


    void
printEntries( FILE* fp )
{
    int     ct  = blkUseCt;
    char**  ppz = papzBlocks;

    if (ct == 0)
        exit( EXIT_FAILURE );

    for (;;) {
        char* pz = *(ppz++);
        if (--ct < 0)
            break;
        fputs( pz, fp );
        free( (void*)pz );
        if (ct > 0)
            fputc( '\n', fp );
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

    if (HAVE_OPT( BASE_NAME )) {
        sprintf( zBase, "-b%s", OPT_ARG( BASE_NAME ));
    }

    else if (HAVE_OPT( DEFS_TO_GET )) {
        char* pzS = OPT_ARG( DEFS_TO_GET );
        strcpy( zBase, "-b" );
        pz = zBase + 2;
        while (isalnum( *pzS ) || (*pzS == '_'))
            *pz++ = *pzS++;
        *pz = NUL;

        if (zBase[2] == NUL) {
            if (getcwd( zSrch, sizeof( zSrch )) == (char*)NULL) {
                fprintf( stderr, "Error %d (%s) on getcwd\n", errno,
                         strerror( errno ));
                exit( EXIT_FAILURE );
            }

            pz = strrchr( zSrch, '/' );
            if (pz == (char*)NULL)
                 pz = zSrch;
            else pz++;
            strcpy( zBase+2, pz );
        }
    }

    if (HAVE_OPT( TEMPLATE )) {
        sprintf( zTempl, "-T%s", OPT_ARG( TEMPLATE ));
	strcpy( zTemplName, OPT_ARG( TEMPLATE ));
    } else {
	strcpy( zTemplName, zBase+2 );
    }

    if (HAVE_OPT( AUTOGEN ))
        switch (WHICH_IDX_AUTOGEN) {
        case INDEX_OPT_OUTPUT:
            if (strcmp( OPT_ARG( OUTPUT ), "-") == 0)
                return stdout;
            return fopen( OPT_ARG( OUTPUT ), "w" );

        case INDEX_OPT_AUTOGEN:
            if (! ENABLED_OPT( AUTOGEN ))
                return stdout;
    
            if (  ( OPT_ARG( AUTOGEN ) != (char*)NULL)
               && (*OPT_ARG( AUTOGEN ) != NUL ))
                pzAutogen = OPT_ARG( AUTOGEN );

            break;
        }

    args[0] = pzAutogen;

    if (HAVE_OPT( TEMPL_DIRS )) {
        sprintf( zSrch, "-L%s", OPT_ARG( TEMPL_DIRS ));
        *pparg++ = zSrch;
    }

    *pparg++ = zBase;

    if (HAVE_OPT( TEMPLATE ))
        *pparg++ = zTempl;

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
