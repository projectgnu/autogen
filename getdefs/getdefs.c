/*
 *  $Id: getdefs.c,v 3.13 2004/02/01 21:26:46 bkorb Exp $
 *
 *    getdefs copyright 1999-2004 Bruce Korb
 *
 *  Time-stamp:        "2003-12-20 10:32:56 bkorb"
 *  Author:            Bruce Korb <bkorb@gnu.org>
 *  Maintainer:        Bruce Korb <bkorb@gnu.org>
 *  Created:           Mon Jun 30 15:35:12 1997
 */

tSCC zBogusDef[] = "Bogus definition:\n%s\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Forward procedure pointers
 */
typedef int (compar_func)(const void *, const void *);
STATIC compar_func compar_text, compar_defname;

/* FORWARD */

STATIC char*
assignIndex( char*  pzOut,  char*  pzDef );

STATIC int
awaitAutogen( void );

STATIC void
buildDefinition(
    char*    pzDef,
    tCC*     pzFile,
    int      line,
    char*    pzOut );

STATIC tSuccess
buildPreamble(
    char**   ppzDef,
    char**   ppzOut,
    tCC*     pzFile,
    int      line );

STATIC int
compar_defname( const void* p1, const void* p2 );

STATIC int
compar_text( const void* p1, const void* p2 );

STATIC void
doPreamble( FILE* outFp );

STATIC void
printEntries( FILE* fp );

STATIC void
processFile( tCC* pzFile );

STATIC void
setFirstIndex( void );

STATIC FILE*
startAutogen( void );

STATIC void
updateDatabase( void );
/* END-FORWARD */

#ifndef HAVE_STRSIGNAL
#  include "compat/strsignal.c"
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   MAIN
 */
int
main( int    argc,
      char** argv )
{
    FILE* outFp;

    optionProcess( &getdefsOptions, argc, argv );
    validateOptions();

    outFp = startAutogen();

    doPreamble( outFp );

    /*
     *  Process each input file
     */
    {
        int    ct  = STACKCT_OPT(  INPUT );
        tCC**  ppz = STACKLST_OPT( INPUT );

        do  {
            processFile( *ppz++ );
        } while (--ct > 0);
    }

    if ((pzIndexText == NULL) && HAVE_OPT( FIRST_INDEX )) {
        qsort( (void*)papzBlocks, blkUseCt, sizeof( char* ), compar_defname );
        setFirstIndex();
    }

    else if (ENABLED_OPT( ORDERING ) && (blkUseCt > 1))
        qsort( (void*)papzBlocks, blkUseCt, sizeof( char* ),
               &compar_text );

    printEntries( outFp );
    fclose( outFp );

    /*
     *  IF output is to a file
     *  THEN set the permissions and modification times
     */
    if (  (WHICH_IDX_AUTOGEN == INDEX_OPT_OUTPUT)
       && (outFp != stdout) )  {
        struct utimbuf tbuf;
        tbuf.actime  = time( (time_t*)NULL );
        tbuf.modtime = modtime + 1;
        utime( OPT_ARG( OUTPUT ), &tbuf );
        chmod( OPT_ARG( OUTPUT ), S_IRUSR|S_IRGRP|S_IROTH);
    }

    /*
     *  IF we are keeping a database of indexes
     *     AND we have augmented the contents,
     *  THEN append the new entries to the file.
     */
    if ((pzIndexText != NULL) && (pzEndIndex != pzIndexEOF))
        updateDatabase();

    if (agPid != -1)
        return awaitAutogen();

    return EXIT_SUCCESS;
}


/*
 *  assignIndex
 */
STATIC char*
assignIndex( char*  pzOut,  char*  pzDef )
{
    char*  pzMatch;
    size_t len = strlen( pzDef );
    int    idx;

    /*
     *  Make the source text all lower case and map
     *  '-', '^' and '_' characters to '_'.
     */
    strtransform( pzDef, pzDef );

    /*
     * IF there is already an entry,
     * THEN put the index into the output.
     */
    pzMatch = strstr( pzIndexText, pzDef );
    if (pzMatch != NULL) {
        pzMatch += len;
        while (isspace( *pzMatch )) pzMatch++;
        while ((*pzOut++ = *pzMatch++) != ']') ;
        return pzOut;
    }

    /*
     *  We have a new entry.  Make sure we have room for it
     *  in our in-memory string
     */
    if (((pzEndIndex - pzIndexText) + len + 64 ) > indexAlloc) {
        char* pz;
        indexAlloc +=  0x1FFF;
        indexAlloc &= ~0x0FFF;
        pz = (char*)realloc( (void*)pzIndexText, indexAlloc );
        if (pz == NULL) {
            fputs( "Realloc of index text failed\n", stderr );
            exit( EXIT_FAILURE );
        }

        /*
         *  IF the allocation moved,
         *  THEN adjust all our pointers.
         */
        if (pz != pzIndexText) {
            pzIndexEOF  = pz + (pzIndexEOF - pzIndexText);
            pzEndIndex  = pz + (pzEndIndex - pzIndexText);
            pzIndexText = pz;
        }
    }

    /*
     *  IF there are no data in our text database,
     *  THEN use default index.
     */
    if (pzEndIndex == pzIndexText)
        idx = OPT_VALUE_FIRST_INDEX;
    else do {
        char* pz = strrchr( pzDef, ' ' );
        *pz = NUL;
        len = strlen( pzDef );

        /*
         *  Find the last entry for the current category of entries
         */
        pzMatch = strstr( pzIndexText, pzDef );
        if (pzMatch == NULL) {
            /*
             *  No entries for this category.  Use default index.
             */
            idx = OPT_VALUE_FIRST_INDEX;
            *pz = ' ';
            break;
        }

        for (;;) {
            char* pzn = strstr( pzMatch + len, pzDef );
            if (pzn == NULL)
                break;
            pzMatch = pzn;
        }

        /*
         *  Skip forward to the '[' character and convert the
         *  number that follows to a long.
         */
        *pz = ' ';
        pzMatch = strchr( pzMatch + len, '[' );
        idx = strtol( pzMatch+1, (char**)NULL, 0 )+1;
    } while (0);

    /*
     *  Add the new entry to our text database and
     *  place a copy of the value into our output.
     */
    pzEndIndex += sprintf( pzEndIndex, "%-40s  [%d]\n", pzDef, idx );
    pzOut += sprintf( pzOut, "[%d]", idx );

    return pzOut;
}


/*
 *  awaitAutogen
 */
STATIC int
awaitAutogen( void )
{
    int  status;
    waitpid( agPid, &status, 0 );
    if (WIFEXITED( status )) {
        status = WEXITSTATUS( status );
        if (status != EXIT_SUCCESS) {
            fprintf( stderr, "ERROR:  %s exited with status %d\n",
                     pzAutogen, status );
        }
        return status;
    }

    if (WIFSIGNALED( status )) {
        status = WTERMSIG( status );
        fprintf( stderr, "ERROR:  %s exited due to %d signal (%s)\n",
                 pzAutogen, status, strsignal( status ));
    }
    else
        fprintf( stderr, "ERROR:  %s exited due to unknown reason %d\n",
                 pzAutogen, status );

    return EXIT_FAILURE;
}


/*
 *  buildDefinition
 */
STATIC void
buildDefinition(
    char*    pzDef,
    tCC*     pzFile,
    int      line,
    char*    pzOut )
{
    tSCC zSrcFile[] = "    %s = '%s';\n";
    tSCC zLineNum[] = "    %s = '%d';\n";

    ag_bool    these_are_global_defs = (*pzDef == '*');
    tSuccess   preamble;
    int        re_res;
    char*      pzNextDef = NULL;
    regmatch_t match[2];

    if (these_are_global_defs) {
        strcpy( pzOut, zGlobal );
        pzOut += sizeof( zGlobal )-1;
        pzOut += sprintf( pzOut, zLineId, line, pzFile );

        pzDef = strchr( pzDef, '\n' );
        preamble = PROBLEM;

    } else {
        preamble = buildPreamble( &pzDef, &pzOut, pzFile, line );
        if (FAILED( preamble )) {
            *pzOut = NUL;
            return;
        }
    }

    /*
     *  FOR each attribute for this entry, ...
     */
    for (;;) {
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
            if (pzNextDef != NULL)
                pzOut = emitDefinition( pzNextDef, pzOut );
            pzNextDef = pzDef = pzDef + match[1].rm_so;
            break;

        case REG_NOMATCH:
            /*
             *  No more attributes.
             */
            if (pzNextDef == NULL) {
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
            char zRER[ MAXNAMELEN ];
            tSCC zErr[] = "error %d (%s) finding `%s' in\n%s\n\n";
            regerror( re_res, &attrib_re, zRER, sizeof( zRER ));
            *pzOut++ = '\n';
            *pzOut++ = '#';
            sprintf( pzOut, zErr, re_res, zRER, zAttribRe, pzDef );
            fprintf( stderr, "getdefs:  %s", zErr );
            return;
        }
        }
    } eachAttrDone:;

    if (these_are_global_defs) {
        *pzOut = NUL;
        return;
    }

    if (HAVE_OPT( COMMON_ASSIGN )) {
        int    ct  = STACKCT_OPT(  COMMON_ASSIGN );
        tCC**  ppz = STACKLST_OPT( COMMON_ASSIGN );
        do  {
            pzOut += sprintf( pzOut, "    %s;\n", *ppz++ );
        } while (--ct > 0);
    }

    if (HAVE_OPT( SRCFILE ))
        pzOut += sprintf( pzOut, zSrcFile, OPT_ARG( SRCFILE ), pzFile );

    if (HAVE_OPT( LINENUM ))
        pzOut += sprintf( pzOut, zLineNum, OPT_ARG( LINENUM ), line );

    /*
     *  IF the preamble had a problem, it is because it could not
     *  emit the final "#endif\n" directive.  Do that now.
     */
    if (HADGLITCH( preamble ))
         strcpy( pzOut, "};\n#endif\n" );
    else strcpy( pzOut, "};\n" );
}


/*
 *  buildPreamble
 */
STATIC tSuccess
buildPreamble(
    char**   ppzDef,
    char**   ppzOut,
    tCC*     pzFile,
    int      line )
{
    char* pzDef = *ppzDef;
    char* pzOut = *ppzOut;

    char  zDefText[ MAXNAMELEN ];
    char* pzDefText = zDefText;
    char  zNameText[ MAXNAMELEN ];
    char* pzNameText = zNameText;
    char* pzIfText   = NULL;

    /*
     *  Copy out the name of the entry type
     */
    *pzDefText++ = '`';
    while (isalnum( *pzDef ) || (*pzDef == '_') || (*pzDef == '.')
          || (*pzDef == '[') || (*pzDef == ']'))
        *pzDefText++ = *pzDef++;

    *pzDefText = NUL;

    pzDef += strspn( pzDef, "* \t" );

    /*
     *  Copy out the name for this entry of the above entry type.
     */
    while (isalnum( *pzDef ) || (*pzDef == '_'))
        *pzNameText++ = *pzDef++;
    *pzNameText = NUL;

    if (  (zDefText[1]  == NUL)
       || (zNameText[0] == NUL) )  {
        fprintf( stderr, zNoData, pzFile, line );
        return FAILURE;
    }

    pzDef += strspn( pzDef, " \t" );

    /*
     *  IF these names are followed by a comma and an "if" clause,
     *  THEN we emit the definition with "#if..."/"#endif" around it
     */
    if (*pzDef == ',') {
        pzDef += strspn( pzDef+1, " \t" )+1;
        if ((pzDef[0] == 'i') && (pzDef[1] == 'f'))
            pzIfText = pzDef;
    }

    pzDef = strchr( pzDef, '\n' );
    if (pzDef == NULL) {
        fprintf( stderr, zNoData, pzFile, line );
        return FAILURE;
    }

    *pzDef = NUL;

    /*
     *  Now start the output.  First, the "#line" directive,
     *  then any "#ifdef..." line and finally put the
     *  entry type name into the output.
     */
    pzOut += sprintf( pzOut, zLineId, line, pzFile );
    if (pzIfText != NULL)
        pzOut += sprintf( pzOut, "#%s\n", pzIfText );
    {
        char*  pz = zDefText+1;
        while (*pz != NUL)
            *pzOut++ = *pz++;
    }

    /*
     *  IF we are indexing the entries,
     *  THEN build the string by which we are indexing
     *       and insert the index into the output.
     */
    if (pzIndexText != NULL) {
        sprintf( pzDefText, "  %s'", zNameText );
        pzOut = assignIndex( pzOut, zDefText );
    }

    /*
     *  Now insert the name with a consistent name string prefix
     *  that we use to locate the sort key later.
     */
    pzOut  += sprintf( pzOut, "%s%s';\n", zNameTag, zNameText );
    *ppzOut = pzOut;
    *ppzDef = pzDef;
    *pzDef  = '\n';  /* restore the newline.  Used in pattern match */

    /*
     *  Returning "PROBLEM" means the caller must emit the "#endif\n"
     *  at the end of the definition.
     */
    return (pzIfText != NULL) ? PROBLEM : SUCCESS;
}


/*
 *  compar_defname
 */
STATIC int
compar_defname( const void* p1, const void* p2 )
{
    tCC* pzS1 = *(tCC* const*)p1;
    tCC* pz1  = strstr( pzS1, zNameTag );
    tCC* pzS2 = *(tCC* const*)p2;
    tCC* pz2  = strstr( pzS2, zNameTag );

    if (pz1 == NULL) {
        if (strncmp( *(tCC* const*)p1, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return -1;

        fprintf( stderr, zBogusDef, *(tCC* const*)p1 );
        exit( EXIT_FAILURE );
    }

    if (pz2 == NULL) {
        if (strncmp( *(tCC* const*)p2, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return 1;

        fprintf( stderr, zBogusDef, *(tCC* const*)p2 );
        exit( EXIT_FAILURE );
    }

    /*
     *  Back up to the name of the definition
     */
    while  ((pz1 > pzS1) && (*--pz1 != '\n'))  ;
    while  ((pz2 > pzS2) && (*--pz2 != '\n'))  ;

    return strcmp( pz1, pz2 );
}


/*
 *  compar_text
 *
 *  merely returns the relative ordering of two input strings.
 *  The arguments are pointers to pointers to NUL-terminated strings.
 *  IF the definiton was mal-formed, an error message was printed
 *  earlier.  When we get here, we wil fail to find the "zNameTag"
 *  string and EXIT_FAILURE.
 */
STATIC int
compar_text( const void* p1, const void* p2 )
{
    char* pz1 = strstr( *(tCC* const*)p1, zNameTag );
    char* pe1;
    char* pz2 = strstr( *(tCC* const*)p2, zNameTag );
    char* pe2;
    int   res;

    if (pz1 == NULL) {
        if (strncmp( *(tCC* const*)p1, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return -1;

        fprintf( stderr, zBogusDef, *(tCC* const*)p1 );
        exit( EXIT_FAILURE );
    }

    if (pz2 == NULL) {
        if (strncmp( *(tCC* const*)p2, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return 1;

        fprintf( stderr, zBogusDef, *(tCC* const*)p2 );
        exit( EXIT_FAILURE );
    }

    pz1 += sizeof( zNameTag )-1;
    pe1 = strchr( pz1, '\'' );

    if (pe1 == NULL) {
        fprintf( stderr, zBogusDef, *(tCC* const*)p1 );
        exit( EXIT_FAILURE );
    }

    pz2 += sizeof( zNameTag )-1;
    pe2 = strchr( pz2, '\'' );

    if (pe2 == NULL) {
        fprintf( stderr, zBogusDef, *(tCC* const*)p2 );
        exit( EXIT_FAILURE );
    }

    *pe1 = *pe2 = NUL;

    /*
     *  We know ordering is enabled because we only get called when
     *  it is enabled.  If the option was also specified, then
     *  we sort without case sensitivity (and we compare '-', '_'
     *  and '^' as being equal as well).  Otherwise, we do a
     *  strict string comparison.
     */
    if (HAVE_OPT( ORDERING ))
         res = streqvcmp( pz1, pz2 );
    else res = strcmp( pz1, pz2 );
    *pe1 = *pe2 = '\'';
    return res;
}


/*
 *  doPreamble
 */
STATIC void
doPreamble( FILE* outFp )
{
    /*
     *  Emit the "autogen definitions xxx;" line
     */
    fprintf( outFp, zAgDef, OPT_ARG( TEMPLATE ));

    if (HAVE_OPT( FILELIST )) {
        tSCC   zFmt[] = "%-12s = '%s';\n";
        tCC*   pzName = OPT_ARG( FILELIST );

        if (pzName == NULL)
            pzName = "infile";

        if (HAVE_OPT( INPUT )) {
            int    ct  = STACKCT_OPT(  INPUT );
            tCC**  ppz = STACKLST_OPT( INPUT );

            do  {
                fprintf( outFp, zFmt, pzName, *ppz++ );
            } while (--ct > 0);
        }

        if (HAVE_OPT( COPY )) {
            int    ct  = STACKCT_OPT(  COPY );
            tCC**  ppz = STACKLST_OPT( COPY );

            do  {
                fprintf( outFp, zFmt, pzName, *ppz++ );
            } while (--ct > 0);
        }
        fputc( '\n', outFp );
    }

    /*
     *  IF there are COPY files to be included,
     *  THEN emit the '#include' directives
     */
    if (HAVE_OPT( COPY )) {
        int    ct  = STACKCT_OPT(  COPY );
        tCC**  ppz = STACKLST_OPT( COPY );
        do  {
            fprintf( outFp, "#include %s\n", *ppz++ );
        } while (--ct > 0);
        fputc( '\n', outFp );
    }

    /*
     *  IF there are global assignments, then emit them
     *  (these do not get sorted, so we write directly now.)
     */
    if (HAVE_OPT( ASSIGN )) {
        int    ct  = STACKCT_OPT(  ASSIGN );
        tCC**  ppz = STACKLST_OPT( ASSIGN );
        do  {
            fprintf( outFp, "%s;\n", *ppz++ );
        } while (--ct > 0);
        fputc( '\n', outFp );
    }
}


/*
 *  loadFile
 */
EXPORT char*
loadFile( tCC* pzFname )
{
    FILE*  fp = fopen( pzFname, "r" FOPEN_BINARY_FLAG );
    int    res;
    char*  pzText;
    char*  pzRead;
    size_t rdsz;

    if (fp == (FILE*)NULL)
        return NULL;
    /*
     *  Find out how much data we need to read.
     *  And make sure we are reading a regular file.
     */
    {
        struct stat stb;
        res = fstat( fileno( fp ), &stb );
        if (res != 0) {
            fprintf( stderr, "error %d (%s) stat-ing %s\n",
                     errno, strerror( errno ), pzFname );
            exit( EXIT_FAILURE );
        }
        if (! S_ISREG( stb.st_mode )) {
            fprintf( stderr, "error file %s is not a regular file\n",
                     pzFname );
            exit( EXIT_FAILURE );
        }
        rdsz = stb.st_size;
        if (rdsz < 16) {
            fprintf( stderr, "Error file %s only contains %d bytes.\n"
                     "\tit cannot contain autogen definitions\n",
                     pzFname, rdsz );
            exit( EXIT_FAILURE );
        }
    }

    /*
     *  Allocate the space we need for the ENTIRE file.
     */
    pzRead = pzText = (char*)malloc( rdsz + 1 );
    if (pzText == NULL) {
        fprintf( stderr, "Error: could not allocate %d bytes\n",
                 rdsz + 1 );
        exit( EXIT_FAILURE );
    }

    /*
     *  Read as much as we can get until we have read the file.
     */
    do  {
        size_t rdct = fread( (void*)pzRead, 1, rdsz, fp );

        if (rdct == 0) {
            fprintf( stderr, "Error %d (%s) reading file %s\n",
                     errno, strerror( errno ), pzFname );
            exit( EXIT_FAILURE );
        }

        pzRead += rdct;
        rdsz   -= rdct;
    } while (rdsz > 0);

    *pzRead = NUL;
    fclose( fp );
    return pzText;
}


/*
 *  printEntries
 */
STATIC void
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


/*
 *  processFile
 */
STATIC void
processFile( tCC* pzFile )
{
    char* pzText = loadFile( pzFile ); /* full text */
    char* pzScan;  /* Scanning Pointer  */
    char* pzDef;   /* Def block start   */
    char* pzNext;  /* start next search */
    char* pzDta;   /* data value        */
    int   lineNo = 1;
    char* pzOut;
    regmatch_t  matches[MAX_SUBMATCH+1];

    if (pzText == NULL) {
        fprintf( stderr, "Error %d (%s) read opening %s\n",
                 errno, strerror( errno ), pzFile );
        exit( EXIT_FAILURE );
    }

    processEmbeddedOptions( pzText );
    pzNext = pzText;

    while ( pzScan = pzNext,
            regexec( &define_re, pzScan, COUNT(matches), matches, 0 ) == 0) {

        static const char zNoEnd[] =
            "Error:  definition in %s at line %d has no end\n";
        static const char zNoSubexp[] =
            "Warning: entry type not found on line %d in %s:\n\t%s\n";

        int  linesInDef = 0;

        /*
         *  Make sure there is a subexpression match!!
         */
        if (matches[1].rm_so == -1) {
            char* pz;
            char  ch = NUL;

            pzDef = pzScan + matches[0].rm_so;
            if (strlen( pzDef ) > 30) {
                pz  = pzDef + 30;
                ch  = *pz;
                *pz = NUL;
            } else
                ch  = NUL;

            fprintf( stderr, zNoSubexp, pzFile, lineNo, pzDef );
            if (ch != NUL)
                *pz = ch;
            continue;
        }

        pzDef = pzScan + matches[0].rm_so + sizeof( "/*=" ) - 1;
        pzNext = strstr( pzDef, "=*/" );
        if (pzNext == NULL) {
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
            if (pzScan++ == NULL)
                break;
            if (pzScan >= pzDef)
                break;
            lineNo++;
        }

        pzOut = pzDta = (char*)malloc( 2 * strlen( pzDef ) + 8000);

        /*
         *  Count the number of lines in the definition itself.
         *  It will find and stop on the "=* /\n" line.
         */
        pzScan = pzDef;
        for (;;) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan++ == NULL)
                break;
            linesInDef++;
        }

        /*
         *  OK.  We are done figuring out where the boundaries of the
         *  definition are and where we will resume our processing.
         */
        buildDefinition( pzDef, pzFile, lineNo, pzOut );
        pzDta   = (char*)realloc( (void*)pzDta, strlen( pzDta ) + 1 );
        lineNo += linesInDef;

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

    free( (void*)pzText );
}


/*
 *  setFirstIndex
 */
STATIC void
setFirstIndex( void )
{
    char    zNm[ 128 ] = "";
    int     nmLn = 1;
    int     ct   = blkUseCt;
    char**  ppz  = papzBlocks;

    if (ct == 0)
        exit( EXIT_FAILURE );

    while (--ct >= 0) {
        char* pzOld = *ppz;
        char* pzNew;
        int   changed = (strneqvcmp( pzOld, zNm, nmLn ) != 0);

        /*
         *  IF the name still matches, then check the following character.
         *  If it is neither whitespace nor an open bracket, then
         *  we have a new entry type.
         */
        if (! changed)
            changed = (! isspace( pzOld[ nmLn ])) && (pzOld[nmLn] != '[');

        if (changed) {
            pzNew = zNm;
            nmLn  = 0;
            while (isalnum( *pzOld ) || (*pzOld == '_') || (*pzOld == '-')) {
                nmLn++;
                *(pzNew++) = *(pzOld++);
            }
            *pzNew = NUL;

            /*
             *  IF the source has specified its own index, then do not
             *  supply our own new one.
             */
            if (*pzOld != '[') {
                pzNew = (char*)malloc( strlen( pzOld ) + nmLn + 10 );
                sprintf( pzNew, "%s[%d]%s", zNm,
                         (int)OPT_VALUE_FIRST_INDEX, pzOld );
                free( (void*)(*ppz) );
                *ppz = pzNew;
            }
        }

        ppz++;
    }
}


/*
 *  startAutogen
 */
STATIC FILE*
startAutogen( void )
{
    char*  pz;
    FILE*  agFp;
    char*  pzBase = NULL;

    /*
     *  Compute the base name.
     *
     *  If an argument was specified, use that without question.
     *  IF a definition pattern is supplied, and it looks like
     *     a normal name, then use that.
     *  If neither of these work, then use the current directory name.
     */
    if (HAVE_OPT( BASE_NAME )) {
        pzBase = malloc( strlen( OPT_ARG( BASE_NAME )) + 3 );
        strcpy( pzBase, "-b" );
        strcpy( pzBase+2, OPT_ARG( BASE_NAME ));
    }
    else {
        /*
         *  IF we have a definition name pattern,
         *  THEN copy the leading part that consists of name-like characters.
         */
        if (HAVE_OPT( DEFS_TO_GET )) {
            tCC* pzS = OPT_ARG( DEFS_TO_GET );
            pzBase = malloc( strlen( pzS ) + 3 );
            strcpy( pzBase, "-b" );

            pz = pzBase + 2;
            while (isalnum( *pzS ) || (*pzS == '_'))
                *pz++ = *pzS++;
            if (pz == pzBase + 2) {
                free( pzBase );
                pzBase = NULL;
            }
            else
                *pz = NUL;
        }

        /*
         *  IF no pattern or it does not look like a name, ...
         */
        if (pzBase == NULL) {
            char zSrch[ MAXPATHLEN ];
            if (getcwd( zSrch, sizeof( zSrch )) == NULL) {
                fprintf( stderr, "Error %d (%s) on getcwd\n", errno,
                         strerror( errno ));
                exit( EXIT_FAILURE );
            }

            pz = strrchr( zSrch, '/' );
            if (pz == NULL)
                 pz = zSrch;
            else pz++;
            pzBase = malloc( strlen( pz ) + 3 );
            strcpy( pzBase, "-b" );
            strcpy( pzBase+2, pz );
        }
    }

    /*
     *  For our template name, we take the argument (if supplied).
     *  If not, then whatever we decided our base name was will also
     *  be our template name.
     */
    if (! HAVE_OPT( TEMPLATE ))
        SET_OPT_TEMPLATE( strdup( pzBase+2 ));

    /*
     *  Now, what kind of output have we?
     *  If it is a file, open it up and return.
     *  If it is an alternate autogen program,
     *  then set it to whatever the argument said it was.
     *  If the option was not supplied, we default to
     *  whatever we set the "pzAutogen" pointer to above.
     */
    if (HAVE_OPT( AUTOGEN ))
        switch (WHICH_IDX_AUTOGEN) {
        case INDEX_OPT_OUTPUT:
        {
            tSCC   zFileFmt[] = " *      %s\n";
            FILE*  fp;

            if (strcmp( OPT_ARG( OUTPUT ), "-") == 0)
                return stdout;

            unlink( OPT_ARG( OUTPUT ));
            fp = fopen( OPT_ARG( OUTPUT ), "w" FOPEN_BINARY_FLAG );
            fprintf( fp, zDne, OPT_ARG( OUTPUT ));

            if (HAVE_OPT( INPUT )) {
                int    ct  = STACKCT_OPT(  INPUT );
                tCC**  ppz = STACKLST_OPT( INPUT );
                do  {
                    fprintf( fp, zFileFmt, *ppz++ );
                } while (--ct > 0);
            }

            fputs( " */\n", fp );
            return fp;
        }

        case INDEX_OPT_AUTOGEN:
            if (! ENABLED_OPT( AUTOGEN ))
                return stdout;

            if (  ( OPT_ARG( AUTOGEN ) != NULL)
               && (*OPT_ARG( AUTOGEN ) != NUL ))
                pzAutogen = OPT_ARG( AUTOGEN );

            break;
        }

    {
        int  pfd[2];

        if (pipe( pfd ) != 0) {
            fprintf( stderr, "Error %d (%s) creating pipe\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );
        }

        agPid = fork();

        switch (agPid) {
        case 0:
            /*
             *  We are the child.  Close the write end of the pipe
             *  and force STDIN to become the read end.
             */
            close( pfd[1] );
            if (dup2( pfd[0], STDIN_FILENO ) != 0) {
                fprintf( stderr, "Error %d (%s) dup pipe[0]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }
            break;

        case -1:
            fprintf( stderr, "Error %d (%s) on fork()\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );

        default:
            /*
             *  We are the parent.  Close the read end of the pipe
             *  and get a FILE* pointer for the write file descriptor
             */
            close( pfd[0] );
            agFp = fdopen( pfd[1], "w" FOPEN_BINARY_FLAG );
            if (agFp == (FILE*)NULL) {
                fprintf( stderr, "Error %d (%s) fdopening pipe[1]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }
            free( pzBase );
            return agFp;
        }
    }

    {
        tCC**  paparg;
        tCC**  pparg;
        int    argCt = 5;

        /*
         *  IF we don't have template search directories,
         *  THEN allocate the default arg counter of pointers and
         *       set the program name into it.
         *  ELSE insert each one into the arg list.
         */
        if (! HAVE_OPT( AGARG )) {
            paparg = pparg = (tCC**)malloc( argCt * sizeof( char* ));
            *pparg++ = pzAutogen;

        } else {
            int    ct  = STACKCT_OPT(  AGARG );
            tCC**  ppz = STACKLST_OPT( AGARG );

            argCt += ct;
            paparg = pparg = (tCC**)malloc( argCt * sizeof( char* ));
            *pparg++ = pzAutogen;

            do  {
                *pparg++ = *ppz++;
            } while (--ct > 0);
        }

        *pparg++ = pzBase;
        *pparg++ = "--";
        *pparg++ = "-";
        *pparg++ = NULL;

#ifdef DEBUG
        fputc( '\n', stderr );
        pparg = paparg;
        for (;;) {
            fputs( *pparg++, stderr );
            if (*pparg == NULL)
                break;
            fputc( ' ', stderr );
        }
        fputc( '\n', stderr );
        fputc( '\n', stderr );
#endif

        execvp( pzAutogen, (char**)(void*)paparg );
        fprintf( stderr, "Error %d (%s) exec of %s %s %s %s\n",
                 errno, strerror( errno ),
                 paparg[0], paparg[1], paparg[2], paparg[3] );
        exit( EXIT_FAILURE );
    }

    return (FILE*)NULL;
}


/*
 *  updateDatabase
 */
STATIC void
updateDatabase( void )
{
    FILE* fp;

    if (chmod( OPT_ARG( ORDERING ), 0666 ) == 0) {
        fp = fopen( OPT_ARG( ORDERING ), "a" FOPEN_BINARY_FLAG );

    } else {
        unlink( OPT_ARG( ORDERING ));
        fp = fopen( OPT_ARG( ORDERING ), "w" FOPEN_BINARY_FLAG );
        pzIndexEOF = pzIndexText;
    }

    if (fp == (FILE*)NULL) {
        fprintf( stderr, "Error %d (%s) opening %s for write/append\n",
                 errno, strerror( errno ), OPT_ARG( ORDERING ));
        exit( EXIT_FAILURE );
    }

    fwrite( pzIndexEOF, (pzEndIndex - pzIndexEOF), 1, fp );
    fclose( fp );
    chmod( OPT_ARG( ORDERING ), 0444 );
}

/* emacs
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of getdefs/getdefs.c */
