/*
 *  $Id: gdinit.c,v 2.1 2001/12/01 20:26:20 bkorb Exp $
 *
 *    getdefs copyright 1999-2001 Bruce Korb
 *
 *  Author:            Bruce Korb <bkorb@gnu.org>
 *  Maintainer:        Bruce Korb <bkorb@gnu.org>
 *  Created:           Sat Dec 1, 2001
 *  Last Modified:     $Date: 2001/12/01 20:26:20 $
 *            by:      Bruce Korb <bkorb@gnu.org>
 */
#include "getdefs.h"

tSCC zNoList[] = "ERROR:  block attr must have name list:\n\t%s\n";

/* FORWARD */

STATIC char*
compressOptionText( char* pzS, char* pzE );

STATIC void
loadStdin( void );
/* END-FORWARD */

/*
 *  compressOptionText
 */
STATIC char*
compressOptionText( char* pzS, char* pzE )
{
    char* pzR = pzS;  /* result      */
    char* pzD = pzS;  /* destination */

    for (;;) {
        char ch;

        if (pzS >= pzE)
            break;

        ch = (*(pzD++) = *(pzS++));

        /*
         *  IF At end of line, skip to next '*'
         */
        if (ch == '\n') {
            while (*pzS != '*') {
                pzS++;
                if (pzS >= pzE)
                    goto compressDone;
            }
        }
    } compressDone:;

    {
        size_t len = (pzD - pzR);
        pzD = malloc( len + 1 );
        if (pzD == NULL) {
            fprintf( stderr, "cannot dup %d byte string\n", pzD - pzR );
            exit( EXIT_FAILURE );
        }

        memcpy( pzD, pzR, len );
        pzD[ len ] = NUL;
    }

    /*
     *  Blank out trailing data.
     */
    while (pzS < pzE)  *(pzS++) = ' ';
    return pzD;
}


/*
 *  fixupSubblockString
 */
EXPORT void
fixupSubblockString( char* pz )
{
    char*   pzString = pz;
    char*   p;
    /*
     *  Make sure we find the '=' separator
     */
    p = strchr( pz, '=' );
    if (p == (char*)NULL) {
        fprintf( stderr, zNoList, pzString );
        USAGE( EXIT_FAILURE );
    }

    /*
     *  NUL the equal char
     */
    *p++ = NUL;
    pz = p;

    /*
     *  Make sure at least one attribute name is defined
     */
    while (isspace( *pz )) pz++;
    if (*pz == NUL) {
        fprintf( stderr, zNoList, pzString );
        USAGE( EXIT_FAILURE );
    }

    for (;;) {
        /*
         *  Attribute names must start with an alpha
         */
        if (! isalpha( *pz )) {
            fprintf( stderr, "ERROR:  attribute names must start "
                     "with an alphabetic character:\n\t%s\n",
                     pzString );
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
}


/*
 *  loadStdin
 *
 *  The input file list is from stdin.
 *
 *  We make some simplifying assumptions:
 *  *ALL* input lines are less than 4096 bytes.  If this is not true,
 *  we may strip some white space in the middle of a line and presume
 *  a comment begins in the middle of a line or we only comment out
 *  the first 4096 bytes of a comment line.  So, rather than all these
 *  problems, we just choke on it.
 */
STATIC void
loadStdin( void )
{
    char z[ 4096 ];
    int    ct  = 0;
    char** ppz = STACKLST_OPT( INPUT );

    while (fgets( z, sizeof( z ), stdin ) != NULL) {
        char* pz = z + strlen( z );

        if (pz[-1] != '\n') {
            tSCC zErr[] =
                "getdefs error: input line not newline terminated\n";
            fputs( zErr, stderr );
            exit( EXIT_FAILURE );
        }

        while ((pz > z) && isspace( pz[-1] ))  pz--;
        *pz = '\0';
        pz = z;
        while (isspace( *pz ))  pz++;
        if ((*pz == '\0') || (*pz == '#'))  continue;
        if (access( pz, R_OK ) != 0)  continue;

        if (ct++ == 0)
            *ppz = strdup( z );  /* replace the "-" */
        else
            SET_OPT_INPUT( strdup( z )); /* if 'strdup' fails, we die later */
    }
}


/*
 *  processEmbeddedOptions
 *
 *  This routine processes the text contained within "/\*==--"
 *  and "=\*\/" as a single option.  If that option is the SUBBLOCK
 *  option, it will need to be massaged for use.
 */
EXPORT void
processEmbeddedOptions( char* pzText )
{
    tSCC zStStr[] = "/*=--";
    tSCC zEndSt[] = "=*/";

    for (;;) {
        char* pzStart = strstr( pzText, zStStr );
        char* pzEnd;
        int   sblct = 0;

        if (pzStart == NULL)
            return;

        if (HAVE_OPT( SUBBLOCK ))
            sblct = STACKCT_OPT(  SUBBLOCK );

        pzEnd = strstr( pzStart, zEndSt );
        if (pzEnd == NULL)
            return;

        pzStart = compressOptionText( pzStart + sizeof( zStStr )-1, pzEnd );

        optionLoadLine( &getdefsOptions, pzStart );

        if (HAVE_OPT( SUBBLOCK ) && (sblct != STACKCT_OPT( SUBBLOCK ))) {
            char**  ppz = STACKLST_OPT( SUBBLOCK );
            fixupSubblockString( ppz[ sblct ] );
        }
        pzText = pzEnd + sizeof( zEndSt );
    }
}


/*
 *  validateOptions
 *
 *  -  Sanity check the options
 *  -  massage the SUBBLOCK options into something
 *     more easily used during the source text processing.
 *  -  compile the regular expressions
 *  -  make sure we can find all the input files and their mod times
 *  -  Set up our entry ordering database (if specified)
 *  -  Make sure we have valid strings for SRCFILE and LINENUM
 *     (if we are to use these things).
 *  -  Initialize the user name characters array.
 */
EXPORT void
validateOptions( void )
{
    /*
     *  Our default pattern is to accept all names following
     *  the '/' '*' '=' character sequence.  We ignore case.
     */
    if ((! HAVE_OPT( DEFS_TO_GET )) || (*OPT_ARG( DEFS_TO_GET ) == NUL)) {
        pzDefPat = "/\\*=([a-z][a-z0-9_]*(\\[[0-9]+\\]){0,1}|\\*)[ \t]+[a-z]";

    } else {
        char*  pz  = OPT_ARG( DEFS_TO_GET );
        size_t len = strlen( pz ) + 16;

        pzDefPat = (char*)malloc( len );
        if (pzDefPat == (char*)NULL) {
            fprintf( stderr, zMallocErr, len, "definition pattern" );
            exit( EXIT_FAILURE );
        }

        /*
         *  IF a pattern has been supplied, enclose it with
         *  the '/' '*' '=' part of the pattern.
         */
        snprintf( pzDefPat, len, "/\\*=(%s)", pz );
    }

    /*
     *  Compile the regular expression that we are to search for
     *  to find each new definition in the source files.
     */
    {
        char zRER[ MAXNAMELEN ];
        static const char zReErr[] =
            "Regex error %d (%s):  Cannot compile reg expr:\n\t%s\n";

        int rerr = regcomp( &define_re, pzDefPat, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &define_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, zRER, pzDefPat );
            exit( EXIT_FAILURE );
        }

        rerr = regcomp( &attrib_re, zAttribRe, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &attrib_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, zRER, zAttribRe );
            exit( EXIT_FAILURE );
        }
    }

    /*
     *  Prepare each sub-block entry so we can parse easily later.
     */
    if (HAVE_OPT( SUBBLOCK )) {
        tSCC    zNoList[] = "ERROR:  block attr must have name list:\n"
                            "\t%s\n";
        int     ct  = STACKCT_OPT(  SUBBLOCK );
        char**  ppz = STACKLST_OPT( SUBBLOCK );

        /*
         *  FOR each SUBBLOCK argument,
         *  DO  condense each name list to be a list of names
         *      separated by a single space and NUL terminated.
         */
        do  {
            fixupSubblockString( *(ppz++) );
        } while (--ct > 0);
    }

    /*
     *  Make sure each of the input files is findable.
     *  Also, while we are at it, compute the output file mod time
     *  based on the mod time of the most recent file.
     */
    {
        int    ct  = STACKCT_OPT(  INPUT );
        char** ppz = STACKLST_OPT( INPUT );
        struct stat stb;

        if ((ct == 1) && (strcmp( *ppz, "-" ) == 0)) {
            loadStdin();
            ct  = STACKCT_OPT(  INPUT );
            ppz = STACKLST_OPT( INPUT );
        }
        do  {
            if (stat( *ppz++, &stb ) != 0)
                break;
            if (! S_ISREG( stb.st_mode )) {
                errno = EINVAL;
                break;
            }

            if (++(stb.st_mtime) > modtime)
                modtime = stb.st_mtime;
        } while (--ct > 0);
        if (ct > 0) {
            fprintf( stderr, "Error %d (%s) stat-ing %s for text file\n",
                     errno, strerror( errno ), ppz[-1] );
            USAGE( EXIT_FAILURE );
        }
    }

    /*
     *  IF the output is to have order AND it is to be based on a file,
     *  THEN load the contents of that file.
     *       IF we cannot load the file,
     *       THEN it must be new or empty.  Allocate several K to start.
     */
    if (   HAVE_OPT( ORDERING )
       && (OPT_ARG( ORDERING ) != (char*)NULL)) {
        tSCC zIndexPreamble[] =
            "# -*- buffer-read-only: t -*- vi: set ro:\n"
            "#\n# DO NOT EDIT THIS FILE - it is auto-edited by getdefs\n";

        pzIndexText = loadFile( OPT_ARG( ORDERING ));
        if (pzIndexText == (char*)NULL) {
            pzIndexText = pzEndIndex  = pzIndexEOF =
                (char*)malloc( 0x4000 );
            indexAlloc = 0x4000;
            pzEndIndex += sprintf( pzEndIndex, "%s", zIndexPreamble );
        } else {
            pzEndIndex  = pzIndexEOF = pzIndexText + strlen( pzIndexText );
            indexAlloc = (pzEndIndex - pzIndexText) + 1;
        }

        /*
         *  We map the name entries to a connonical form.
         *  By default, everything is mapped to lower case already.
         *  This call will map these three characters to '_'.
         */
        strequate( "_-^" );
    }

    if (OPT_ARG( SRCFILE ) == (char*)NULL)
        OPT_ARG( SRCFILE ) = "srcfile";

    if (OPT_ARG( LINENUM ) == (char*)NULL)
        OPT_ARG( LINENUM ) = "linenum";

    {
        tSCC zAgNameChars[] = "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" "_-^";
        tSCC zUserNameChs[] = ":.$%*!~<>&@";
        tCC* p = zAgNameChars;

        while (*p)
            zUserNameCh[(unsigned)(*p++)] = 3;

        p = zUserNameChs;
        while (*p)
            zUserNameCh[(unsigned)(*p++)] = 1;
    }
}


/* emacs
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of gdinit.c */
