
/*
 *  agLex.c
 *  $Id: defLex.c,v 1.1.1.1 1999/10/14 00:33:53 bruce Exp $
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

extern YYSTYPE yylval;
static YYSTYPE lastToken;

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

STATIC char* assembleString( char* );
STATIC char* assembleName( char*, YYSTYPE* );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   LEXICAL SCANNER
 */
    int
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

    case '=':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
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

    case '(':
    {
        char* pzRes = pCurCtx->pzScan;
        char* pzEnd = (char*)skipScheme( pzRes, pzRes + strlen( pzRes ));
        char  endCh = *pzEnd;
        int   expLen;
        SCM   str;

        /*
         *  NUL terminate the Scheme expression, run it, then restore
         *  the NUL-ed character.
         */
        *pzEnd = NUL;
        procState = PROC_STATE_GUILE_PRELOAD;
        str = gh_eval_str( pzRes );
        procState = PROC_STATE_LOAD_DEFS;
        expLen = pzEnd - pzRes;
        pCurCtx->pzScan = pzEnd;
        *pzEnd = endCh;

        /*
         *  IF we have a string result, then try to overwrite the
         *  original expression with the result of the expression.
         *  Resort to allocation if the result is larger, however.
         */
        if (gh_string_p( str )) {
            pzEnd = SCM_CHARS( str );
            if (strlen( pzEnd ) < expLen)
                strcpy( pzRes, pzEnd );
            else {
                pzRes = strdup( pzEnd );
            }
        }

        /*
         *  IF the result is a character, then it is certain to fit.
         */
        else if (gh_char_p( str )) {
            pzRes[0] = gh_scm2char( str );
            pzRes[1] = NUL;
        }

        /*
         *  IF the result is a number, it might fit.  Pick a size that
         *  is sure to accommodate the formatted number.
         */
        else if (gh_number_p( str )) {
            if (expLen > 16)
                sprintf( pzRes, "%ld", gh_scm2long( str ));
            else
                pzRes = asprintf( "%ld", gh_scm2long( str ));
        }

        /*
         *  OTHERWISE, the empty string.
         */
        else
            *pzRes = NUL;

        lastToken = TK_STRING;
        yylval = (YYSTYPE)pzRes;
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
        pCurCtx->pzScan = assembleName( pCurCtx->pzScan, &lastToken );
        break;
    }   /* switch (*pCurCtx->pzScan) */

    return lastToken;

NUL_error:

    fprintf( stderr, "%s ERROR:  unterminated quote in definition "
             "in %s on line %d\n", pzProg,
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



    void
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
        fprintf( stderr, "`%c' (%d)\n", lastToken );
    }

    fprintf( stderr, "\n[[...<error-text>]] %s\n\n", pCurCtx->pzScan );
    AG_ABEND;
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
             *  Now disallow entirely characters we use as tokens
             */
            zNameChars[ (unsigned)'#' ] = 0;
            zNameChars[ (unsigned)'=' ] = 0;
            zNameChars[ (unsigned)'{' ] = 0;
            zNameChars[ (unsigned)'}' ] = 0;
            zNameChars[ (unsigned)'[' ] = 0;
            zNameChars[ (unsigned)']' ] = 0;
            zNameChars[ (unsigned)';' ] = 0;
            zNameChars[ (unsigned)'"' ] = 0;
            zNameChars[ (unsigned)'`' ] = 0;
            zNameChars[ (unsigned)'\'' ] = 0;
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
                fprintf( stderr, "Invalid input char '%c' in %s on line %d\n",
                         *pzScan, pCurCtx->pzFileName, pCurCtx->lineNo );
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
