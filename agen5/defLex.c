
/*
 *  $Id: defLex.c,v 1.13 2000/04/06 17:11:14 bkorb Exp $
 *  This module scans the template variable declarations and passes
 *  tokens back to the parser.
 */

/*
 *  AutoGen copyright 1992-1999 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

#include <streqv.h>

#include "autogen.h"
#include "defParse.h"
#include "expGuile.h"

tSCC zSchemedefFile[]   = "schemedef.scm";
ag_bool schemedefLoaded = AG_FALSE;

extern YYSTYPE yylval;
static YYSTYPE lastToken;

tSCC zErrMsg[] = "%s Error:  %s in %s on line %d\n";

/*
 *  This keyword table must match those found in agParse.y.
 *  You will find them in a %token statement that follows
 *  a comment  "Keywords"
 */
#define KEYWORD_TABLE                           \
  _KW_( AUTOGEN )                               \
  _KW_( DEFINITIONS )

#define _KW_(w) tSCC z ## w [] = #w;
KEYWORD_TABLE
#undef _KW_

#define _KW_(w) z ## w,
tSCC*  apzKeywords[] = { KEYWORD_TABLE };
#undef _KW_

#define _KW_(w) TK_ ## w,
int    aKeywordTkn[] = { KEYWORD_TABLE };
#undef _KW_

#define KEYWORD_CT  (sizeof( apzKeywords ) / sizeof( apzKeywords[0] ))

#define ERROR  (-1)
#define FINISH (-1)

STATIC void  loadScheme( void );
STATIC void  alist_to_autogen_def( void );
STATIC char* assembleName( char* pzScan, YYSTYPE* pRetVal );
STATIC char* assembleString( char* pzScan );
STATIC char* assembleHereString( char* pzScan );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   LEXICAL SCANNER
 */
    EXPORT int
yylex( void )
{
    lastToken = ERROR;

scanAgain:
    /*
     *  Start the process of locating a token.
     *  We branch here after skipping over a comment
     *  or processing a directive (which may change our context).
     */
    if (isspace( *pCurCtx->pzScan )) {
        char* pz = pCurCtx->pzScan;
        if (*pz == '\n')
            pCurCtx->lineNo++;
        *(pz++) = NUL;

        /*
         *  This ensures that any names found previously
         *  are NUL terminated.
         */
        while (isspace(*pz)) {
            if (*pz == '\n')
                pCurCtx->lineNo++;
            pz++;
        }
        pCurCtx->pzScan = pz;
    }

    switch (*pCurCtx->pzScan) {
    case NUL:
        /*
         *  IF we are not inside an include context,
         *  THEN go finish.
         */
        if (pCurCtx->pCtx == (tScanCtx*)NULL)
            goto lex_done;

        /*
         *  Pop off an include context and resume
         *  from the including file.
         */
        {
            tScanCtx* pCX = pCurCtx;
            pCurCtx = pCurCtx->pCtx;
            AGFREE( (void*)pCX->pzFileName );
            pCX->pCtx = pDoneCtx;
            pDoneCtx  = pCX;
        }
        goto scanAgain;

    case '#':
    {
        extern char* processDirective( char* );
        char*  pz;

        /*
         *  NB:  This character and all others that we treat specially
         *       here must be disallowed as "OTHER_NAME" characters below
         */
        pz = processDirective( pCurCtx->pzScan+1 );
        /*
         *  Advance past any leading blanks
         */
        while (isspace( *pz )) {
            if (*pz == '\n')
                pCurCtx->lineNo++;
            pz++;
        }

        pCurCtx->pzScan = pz;
        goto scanAgain;
    }

    case '{':
    case '=':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
        /*
         *  The character itself is the token value.
         *  We NUL it out in the text to ensure that any
         *  names encountered are NUL terminated.
         */
        lastToken = *pCurCtx->pzScan;
        *(pCurCtx->pzScan++) = NUL;

        /*
         *  IF the next character is an out of context preprocessing directive
         *  THEN call "assembleName" now to generate an error message.
         */
        if (*pCurCtx->pzScan == '#')
            pCurCtx->pzScan = assembleName( pCurCtx->pzScan, &lastToken );
        break;

    case '\'':
    case '"':
    {
        char* pz = assembleString( pCurCtx->pzScan );
        if (pz == (char*)NULL)
            goto NUL_error;

        lastToken = TK_STRING;
        pCurCtx->pzScan = pz;
        break;
    }

    case '<':
    {
        char* pz;
        if (pCurCtx->pzScan[1] != '<')
            goto BrokenToken;

        pz = assembleHereString( pCurCtx->pzScan + 2 );
        if (pz == (char*)NULL) {
            lastToken = ERROR;
            return ERROR;
        }

        lastToken = TK_STRING;
        pCurCtx->pzScan = pz;
        break;
    }

    case '(':
        loadScheme();
        break;

    case '\\':
        if (strncmp( pCurCtx->pzScan+1, "'(", 2) == 0) {
            alist_to_autogen_def();
            goto scanAgain;

        } else {
            char* pz = strchr( pCurCtx->pzScan, ';' );

            for (;;) {
                if (pz == (char*)NULL) {
                    pz = pCurCtx->pzScan + strlen( pCurCtx->pzScan );
                    break;
                }
                if (isspace( pz[1] )) {
                    *pz = NUL;
                    pz[1] = ';';
                    break;
                }
                pz = strchr( pz+1, ';' );
            }

            lastToken = TK_STRING;
            yylval = (YYSTYPE)pz;
            break;
        }

    case '`':
    {
        char*   pz = assembleString( pCurCtx->pzScan );

        if (pz == (char*)NULL)
            goto NUL_error;

        pCurCtx->pzScan = pz;

        lastToken = TK_STRING;
        pz = runShell( (const char*)yylval );
        if (pz == (char*)NULL)
            goto scanAgain;
        TAGMEM( pz, "shell definition string" );
        yylval = (YYSTYPE)pz;
        break;
    }

    case '/':
        /*
         *  Allow for a comment, C or C++ style
         */
        switch (pCurCtx->pzScan[1]) {
        case '*':
        {
            char* pz = strstr( pCurCtx->pzScan+2, "*/" );
            if (pz != (char*)NULL) {
                char* p = pCurCtx->pzScan+1;
                for (;;) {
                    p = strchr( p+1, '\n' );
                    if ((p == (char*)NULL) || (p > pz))
                        break;
                    pCurCtx->lineNo++;
                }
                pCurCtx->pzScan = pz+2;
                goto scanAgain;
            }
            break;
        }
        case '/':
        {
            char* pz = strchr( pCurCtx->pzScan+2, '\n' );
            if (pz != (char*)NULL) {
                pCurCtx->pzScan = pz+1;
                goto scanAgain;
            }
            break;
        }
        }
        /* FALLTHROUGH */ /* to Invalid input char */

    default:
    BrokenToken:
        pCurCtx->pzScan = assembleName( pCurCtx->pzScan, &lastToken );
        break;
    }   /* switch (*pCurCtx->pzScan) */

    return lastToken;

NUL_error:

    fprintf( stderr, zErrMsg, pzProg, "unterminated quote in definition",
             pCurCtx->pzFileName, pCurCtx->lineNo );
    lastToken = ERROR;
    return ERROR;

lex_done:
    {
        tSCC zDone[] = "";

        /*
         *  First time through, return the TK_END token.
         *  Second time through, we really finish.
         */
        if (pCurCtx->pzScan == zDone) {
            pCurCtx->pCtx = pDoneCtx;
            pDoneCtx      = pCurCtx;

            return FINISH;
        }

        pCurCtx->pzScan = (char*)zDone;
        return TK_END;
    }
}



    EXPORT void
yyerror( char* s )
{
    fprintf( stderr, "%s:  in %s on line %d\n    token in error:  ",
             s, pCurCtx->pzFileName, pCurCtx->lineNo );

    if (strlen( pCurCtx->pzScan ) > 64 )
        pCurCtx->pzScan[64] = NUL;

    switch (lastToken) {
    case TK_AUTOGEN:
        fputs( "AUTOGEN\n", stderr );
        break;

    case TK_DEFINITIONS:
        fputs( "DEFINITIONS\n", stderr );
        break;

    case TK_END:
        fputs( "END\n", stderr );
        break;

    case TK_VAR_NAME:
        fprintf( stderr, "VAR_NAME %s\n", (char*)yylval );
        break;

    case TK_OTHER_NAME:
        fprintf( stderr, "OTHER_NAME %s\n", (char*)yylval );
        break;

    case TK_STRING:
        fprintf( stderr, "STRING %s\n", (char*)yylval );
        break;

    case TK_NUMBER:
        fprintf( stderr, "NUMBER %s\n", (char*)yylval );
        break;

    default:
        fprintf( stderr, "`%1$c' (%1$d)\n", lastToken );
    }

    fprintf( stderr, "\n[[...<error-text>]] %s\n\n", pCurCtx->pzScan );
    AG_ABEND;
}


    STATIC void
loadScheme( void )
{
    char*    pzText    = pCurCtx->pzScan;
    char*    pzEnd     = (char*)skipScheme( pzText, pzText + strlen( pzText ));
    char     endCh     = *pzEnd;
    int      schemeLen = (pzEnd - pzText);
    ag_bool  mustFree  = AG_FALSE;
    SCM      res;

    /*
     *  NUL terminate the Scheme expression, run it, then restore
     *  the NUL-ed character.
     */
    *pzEnd = NUL;
    procState = PROC_STATE_GUILE_PRELOAD;
    res = gh_eval_str( pzText );
    procState = PROC_STATE_LOAD_DEFS;
    *pzEnd = endCh;

    pCurCtx->pzScan = pzEnd;
    pzEnd = resolveSCM( res, &mustFree );
    if (strlen( pzEnd ) >= schemeLen) {
        if (! mustFree)
            yylval = (YYSTYPE)strdup( pzEnd );
        else
            yylval = (YYSTYPE)pzEnd;
    }

    else {
        strcpy( pzText, pzEnd );
        if (mustFree)
            AGFREE( (void*)pzEnd );
        yylval = (YYSTYPE)pzText;
    }

    lastToken = TK_STRING;
}

/*
 *  process a single scheme expression, yielding text that gets processed
 *  into AutoGen definitions.
 */
    STATIC void
alist_to_autogen_def( void )
{
    tSCC   zSchemeText[] = "Scheme Computed Definitions";
    tSCC   zWrap[] = "(alist->autogen-def %s)";

    char*  pzText  = ++(pCurCtx->pzScan);
    char*  pzEnd   = (char*)skipScheme( pzText, pzText + strlen( pzText ));

    SCM    res;
    tScanCtx*  pCtx;

    if (! schemedefLoaded) {
        SET_OPT_LOAD_SCHEME( (char*)zSchemedefFile );
        schemedefLoaded = AG_TRUE;
    }

    /*
     *  Wrap the scheme expression with the `alist->autogen-def' function
     */
    {
        char endCh = *pzEnd;
        *pzEnd = NUL;
        pzText = asprintf( zWrap, pzText );
        *pzEnd = endCh;
    }

    /*
     *  NUL terminate the Scheme expression, run it, then restore
     *  the NUL-ed character.
     */
    procState = PROC_STATE_GUILE_PRELOAD;
    res       = gh_eval_str( pzText );
    procState = PROC_STATE_LOAD_DEFS;
    pCurCtx->pzScan = pzEnd;
    AGFREE( (void*)pzText );

    /*
     *  The result *must* be a string, or we choke.
     */
    if (! gh_string_p( res )) {
        tSCC zEr[] = "Error:  Scheme expression does not yield string:\n";
        fputs( zEr, stderr );
        AG_ABEND;
    }

    /*
     *  Now, push the resulting string onto the input stack.
     */
    pzText = SCM_CHARS( res );
    pCtx = (tScanCtx*)AGALOC( sizeof( tScanCtx ) + 4 + strlen( pzText ));
    if (pCtx == (tScanCtx*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 sizeof( tScanCtx ), "scheme expression" );
        AG_ABEND;
    }

    /*
     *  Link the new scan data into the context stack
     */
    pCtx->pCtx  = pCurCtx;
    pCurCtx     = pCtx;

    /*
     *  Set up the rest of the context structure
     */
    AGDUPSTR( pCtx->pzFileName, zSchemeText );
    pCtx->pzScan = \
    pCtx->pzData = (char*)(pCtx+1);
    pCtx->lineNo = 0;
    strcpy( pCtx->pzScan, pzText );
    /*
     *  At this point, the next token will be obtained
     *  from the newly allocated context structure.
     *  When empty, input will resume from the '}' that we
     *  left as the next input token in the old context.
     */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  It may be a number, a name, a keyword or garbage.
 *  Figure out which.
 */
    STATIC char*
assembleName( char* pzScan, YYSTYPE* pRetVal )
{
    /*
     *  Check for a number.
     *  Scan it in and advance "pzScan".
     */
    if (  isdigit( *pzScan )
       || (  (*pzScan == '-')
          && isdigit( pzScan[1] )
       )  )  {
        yylval = (YYSTYPE)pzScan;
        (void)strtol( pzScan, &pzScan, 0 );
        *pRetVal = TK_NUMBER;
        return pzScan;
    }

    {
        static unsigned char zNameChars[ 256 ];
        unsigned char* pz = (unsigned char*)pzScan;

        if (zNameChars[ (unsigned)'a' ] == 0) {
            /*
             *  Default to accepting as "OTHER_NAME" all characters
             */
            u_int  idx = ((unsigned)' ') + 1;
            do  {
                zNameChars[ idx ] = 2;
            } while (++idx <= (unsigned)'~');

            /*
             *  Now change alphanumerics and '_' to be VAR_NAME chars.
             */
            idx = (unsigned)'a';
            do  {
                zNameChars[ idx ] = 1;
            } while (++idx <= (unsigned)'z');
            idx = (unsigned)'A';
            do  {
                zNameChars[ idx ] = 1;
            } while (++idx <= (unsigned)'Z');
            idx = (unsigned)'0';
            do  {
                zNameChars[ idx ] = 1;
            } while (++idx <= (unsigned)'9');
            zNameChars[ (unsigned)'_' ] = 1;

            /*
             *  Now disallow entirely characters we use specially
             */
            zNameChars[ (unsigned)'"' ] = 0;
            zNameChars[ (unsigned)'#' ] = 0;
            zNameChars[ (unsigned)'(' ] = 0;
            zNameChars[ (unsigned)')' ] = 0;
            zNameChars[ (unsigned)',' ] = 0;
            zNameChars[ (unsigned)';' ] = 0;
            zNameChars[ (unsigned)'<' ] = 0;
            zNameChars[ (unsigned)'=' ] = 0;
            zNameChars[ (unsigned)'>' ] = 0;
            zNameChars[ (unsigned)'[' ] = 0;
            zNameChars[ (unsigned)'\''] = 0;
            zNameChars[ (unsigned)']' ] = 0;
            zNameChars[ (unsigned)'`' ] = 0;
            zNameChars[ (unsigned)'{' ] = 0;
            zNameChars[ (unsigned)'}' ] = 0;
        }

        /*
         *  Skip over VAR_NAME characters
         */
        while (zNameChars[ *pz ] == 1) pz++;

        /*
         *  IF the next character terminates the token,
         *  THEN see if we got any characters at all
         *  ELSE skip over the rest of the OTHER_NAME
         */
        if (zNameChars[ *pz ] == 0) {
            if (pz == (unsigned char*)pzScan) {
                fprintf( stderr, "%s Error: Invalid input char '%c' "
                         "in %s on line %d\n", pzProg, *pzScan,
                         pCurCtx->pzFileName, pCurCtx->lineNo );
                *pRetVal = FINISH;
                return pzScan;
            }
            *pRetVal = TK_VAR_NAME;
        } else {
            *pRetVal = TK_OTHER_NAME;
            while (zNameChars[ *pz ] != 0) pz++;
        }

        /*
         *  Return a NAME token, maybe.
         *  If the name is actually a keyword,
         *  we will return that token code instead.
         */
        yylval   = (YYSTYPE)pzScan;
        pzScan   = (char*)pz;
    }

    /*
     *  Now scan the keyword table.
     */
    if (*pRetVal == TK_VAR_NAME) {
        char sv_ch = *pzScan;  /* preserve the following character */
        int  kw_ix = 0;
        *pzScan = NUL;         /* NUL terminate the name           */

        do  {
            if (streqvcmp( apzKeywords[ kw_ix ], (char*)yylval ) == 0) {
                /*
                 *  Return the keyword token code instead of TK_NAME
                 */
                *pRetVal = aKeywordTkn[ kw_ix ];
                break;
            }
        } while (++kw_ix < KEYWORD_CT);

        *pzScan = sv_ch;         /* restore the following character  */
    }

    return pzScan;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  A quoted string has been found.
 *  Find the end of it and compress any escape sequences.
 */
    STATIC char*
assembleHereString( char* pzScan )
{
    ag_bool  trimTabs = AG_FALSE;
    char     zMark[ 64 ];
    int      markLen = 0;
    char*    pzDest;

    /*
     *  See if we are to strip leading tab chars
     */
    if (*pzScan == '-') {
        trimTabs = AG_TRUE;
        pzScan++;
    }

    /*
     *  Skip white space up to the marker or EOL
     */
    while (isspace( *pzScan )) {
        if (*pzScan++ == '\n') {
            fprintf( stderr, zErrMsg, pzProg, "HereString missing the mark",
                     pCurCtx->pzFileName, pCurCtx->lineNo );
            return (char*)NULL;
        }
    }

    /*
     *  Copy the marker, noting its length
     */
    {
        char* pz = zMark;
        while (ISNAMECHAR( *pzScan )) {
            if (++markLen >= sizeof(zMark)) {
                fprintf( stderr, zErrMsg, pzProg,
                         "HereString mark over 63 chars",
                         pCurCtx->pzFileName, pCurCtx->lineNo );
                return (char*)NULL;
            }

            *(pz++) = *(pzScan++);
        }
        if (markLen == 0) {
            fprintf( stderr, zErrMsg, pzProg, "HereString missing the mark",
                     pCurCtx->pzFileName, pCurCtx->lineNo );
            return (char*)NULL;
        }
        *pz = NUL;
    }

    pzDest = pzScan;
    yylval = (YYSTYPE)pzDest;

    /*
     *  Skip forward to the EOL after the marker.
     */
    pzScan = strchr( pzScan, '\n' );
    if (pzScan == (char*)NULL) {
        fprintf( stderr, zErrMsg, pzProg, "Unterminated HereString",
                 pCurCtx->pzFileName, pCurCtx->lineNo );
        return (char*)NULL;
    }

    /*
     *  And skip the first new line + conditionally skip tabs
     */
    pzScan++;

    if (trimTabs)
        while (*pzScan == '\t')  ++pzScan;

    /*
     *  FOR as long as the text does not match the mark
     *       OR it matches but is a substring
     *  DO copy characters
     */
    while (  (strncmp( pzScan, zMark, markLen ) != 0)
          || ISNAMECHAR( pzScan[ markLen ]) )  {

        for (;;) {
            switch (*(pzDest++) = *(pzScan++)) {
            case '\n':
                goto lineDone;

            case NUL:
                fprintf( stderr, zErrMsg, pzProg, "Unterminated HereString",
                         pCurCtx->pzFileName, pCurCtx->lineNo );
                return (char*)NULL;
            }
        } lineDone:;

        if (trimTabs)
            while (*pzScan == '\t')  ++pzScan;
    } /* while strncmp ... */

    /*
     *  pzDest may still equal yylval, if no data were copied
     */
    if (pzDest > (char*)yylval)
         pzDest[-1] = NUL;
    else pzDest[0]  = NUL;

    return pzScan + markLen;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  A quoted string has been found.
 *  Find the end of it and compress any escape sequences.
 */
    STATIC char*
assembleString( char* pzScan )
{
    char  q = *(pzScan++);

    /*
     *  It is a quoted string.  Process the escape characters
     *  (in the set "abfnrtv") and make sure we find a closing quote.
     */
    char* pzD = pzScan;
    char* pzS = pzScan;

    yylval = (YYSTYPE)pzD;

    for (;;) {
        /*
         *  IF the next character is the quote character,
         *  THEN we may end the string.  We end it unless
         *  the next non-blank character *after* the string
         *  happens to also be a quote.  If it is, then
         *  we will change our quote character to the new
         *  quote character and continue condensing text.
         */
        while (*pzS == q) {
            *pzD = NUL;
            while (isspace(*(++pzS)))
                if (*pzS == '\n')
                    pCurCtx->lineNo++;

            /*
             *  IF the next character is a quote character,
             *  THEN we will concatenate the strings.
             */
            switch (*pzS) {
            case '"':
            case '\'':
                break;

            case '#':
                /*
                 *  IF the next character is a directive,
                 *  THEN ensure it is in the first column!!
                 */
                if (pzS[-1] != '\n')
                    return assembleName( pzS, &yylval );
                /* FALLTHROUGH */

            default:
                return pzS;
            }
            q = *(pzS++);
        }

        switch (*(pzD++) = *(pzS++)) {
        case NUL:
            return (char*)NULL;

        case '\n':
            pCurCtx->lineNo++;
            break;

        case '\\':
            /*
             *  IF we are escaping a new line,
             *  THEN drop both the escape and the newline from
             *       the result string.
             */
            if (*pzS == '\n') {
                pzS++;
                pzD--;
                pCurCtx->lineNo++;
            }

            /*
             *  ELSE IF the quote character is '"' or '`',
             *  THEN we do the full escape character processing
             */
            else if (q != '\'') {
                int ct = doEscapeChar( pzS, pzD-1 );
                if (ct == 0)
                    return (char*)NULL;

                pzS += ct;
            }     /* if (q != '\'')                  */

            /*
             *  OTHERWISE, we only process "\\", "\'" and "\#" sequences.
             *  The latter only to easily hide preprocessing directives.
             */
            else switch (*pzS) {
            case '\\':
            case '\'':
            case '#':
                pzD[-1] = *pzS++;
            }
        }     /* switch (*(pzD++) = *(pzS++))    */
    }         /* for (;;)                        */
}
/* end of defLex.c */
