
/*
 *  $Id: defLex.c,v 3.21 2003/05/31 23:15:06 bkorb Exp $
 *  This module scans the template variable declarations and passes
 *  tokens back to the parser.
 */

/*
 *  AutoGen copyright 1992-2003 Bruce Korb
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
        if (pCurCtx->pCtx == NULL)
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
        char* pz = processDirective( pCurCtx->pzScan+1 );
        /*
         *  Ensure that the compiler doesn't try to save a copy of
         *  "pCurCtx" in a register.  It must be reloaded from memory.
         */
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
        lastToken = *(pCurCtx->pzScan);
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
        if (pz == NULL)
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
        if (pz == NULL) {
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
                if (pz == NULL) {
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

        if (pz == NULL)
            goto NUL_error;

        pCurCtx->pzScan = pz;

        if (pzShellProgram == NULL)
            pzShellProgram = getDefine( zShellEnv );

        lastToken = TK_STRING;
        pz = runShell( (const char*)yylval );
        if (pz == NULL)
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
            if (pz != NULL) {
                char* p = pCurCtx->pzScan+1;
                for (;;) {
                    p = strchr( p+1, '\n' );
                    if ((p == NULL) || (p > pz))
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
            if (pz != NULL) {
                pCurCtx->pzScan = pz+1;
                pCurCtx->lineNo++;
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

    AG_ABEND( aprf( zErrMsg, pzProg, "unterminated quote in definition",
                    pCurCtx->pzFileName, pCurCtx->lineNo ));
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
    tSCC zVN[] = "VAR_NAME %s\n";
    tSCC zON[] = "OTHER_NAME %s\n";
    tSCC zSt[] = "STRING %s\n";
    tSCC zNo[] = "NUMBER %s\n";
    tSCC zDf[] = "`%1$c' (%1$d)\n";

    char* pz;

    if (strlen( pCurCtx->pzScan ) > 64 )
        pCurCtx->pzScan[64] = NUL;

    switch (lastToken) {
    case TK_AUTOGEN:     pz = "AUTOGEN\n";     break;
    case TK_DEFINITIONS: pz = "DEFINITIONS\n"; break;
    case TK_END:         pz = "END\n";         break;
    case TK_VAR_NAME:    pz = aprf( zVN, (char*)yylval ); break;
    case TK_OTHER_NAME:  pz = aprf( zON, (char*)yylval ); break;
    case TK_STRING:      pz = aprf( zSt, (char*)yylval ); break;
    case TK_NUMBER:      pz = aprf( zNo, (char*)yylval ); break;
    default:             pz = aprf( zDf, lastToken );     break;
    }
    AG_ABEND( aprf( "%s:  in %s on line %d\n"
                    "\ttoken in error:  %s\n"
                    "\t[[...<error-text>]] %s\n\n"
                    "Likely causes:  a mismatched quote, a value that needs "
                    "quoting,\n\t\tor a missing semi-colon\n",
                    s, pCurCtx->pzFileName, pCurCtx->lineNo, pz,
                    pCurCtx->pzScan ));
}


STATIC void
loadScheme( void )
{
    char*    pzText    = pCurCtx->pzScan;
    char*    pzEnd     = (char*)skipScheme( pzText, pzText + strlen( pzText ));
    char     endCh     = *pzEnd;
    int      schemeLen = (pzEnd - pzText);
    SCM      res;

    /*
     *  NUL terminate the Scheme expression, run it, then restore
     *  the NUL-ed character.
     */
    *pzEnd = NUL;
    procState = PROC_STATE_GUILE_PRELOAD;
    res = ag_scm_c_eval_string_from_file_line(
        pzText, pCurCtx->pzFileName, pCurCtx->lineNo );
    procState = PROC_STATE_LOAD_DEFS;
    *pzEnd = endCh;

    pCurCtx->pzScan = pzEnd;
    pzEnd = resolveSCM( res );
    if (strlen( pzEnd ) >= schemeLen) {
        AGDUPSTR( pzEnd, pzEnd, "SCM Result" );

        yylval = (YYSTYPE)pzEnd;
    }

    else {
        /*
         *  We know the result is smaller than the source.  Copy in place.
         */
        strcpy( pzText, pzEnd );
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
    int    res_len;
    tScanCtx*  pCtx;

    /*
     *  Wrap the scheme expression with the `alist->autogen-def' function
     */
    {
        char endCh = *pzEnd;
        *pzEnd = NUL;
        pzText = aprf( zWrap, pzText );
        *pzEnd = endCh;
    }

    /*
     *  NUL terminate the Scheme expression, run it, then restore
     *  the NUL-ed character.
     */
    procState = PROC_STATE_GUILE_PRELOAD;
    res = ag_scm_c_eval_string_from_file_line(
        pzText, pCurCtx->pzFileName, pCurCtx->lineNo );

    /*
     *  The result *must* be a string, or we choke.
     */
    if (! gh_string_p( res )) {
        tSCC zEr[] = "Scheme definition expression does not yield string:\n";
        AG_ABEND( zEr );
    }

    res_len   = SCM_LENGTH( res );
    procState = PROC_STATE_LOAD_DEFS;
    pCurCtx->pzScan = pzEnd;
    AGFREE( (void*)pzText );

    /*
     *  Now, push the resulting string onto the input stack
     *  and link the new scan data into the context stack
     */
    pCtx = (tScanCtx*)AGALOC( sizeof(tScanCtx) + 4 + res_len, "lex scan ctx" );
    pCtx->pCtx  = pCurCtx;
    pCurCtx     = pCtx;

    /*
     *  Set up the rest of the context structure
     */
    AGDUPSTR( pCtx->pzFileName, zSchemeText, "scheme text" );
    pCtx->pzScan = \
    pCtx->pzData = (char*)(pCtx+1);
    pCtx->lineNo = 0;
    memcpy( (void*)(pCtx->pzScan), (void*)SCM_CHARS( res ), res_len );
    pCtx->pzScan[ res_len ] = NUL;

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
                zNameChars[ idx ] = ISNAMECHAR(idx) ? 1 : 2;
            } while (++idx <= (unsigned)'~');

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
            if (pz == (unsigned char*)pzScan)
                AG_ABEND( aprf( "%s Error: Invalid input char '%c' "
                                "in %s on line %d\n", pzProg, *pzScan,
                                pCurCtx->pzFileName, pCurCtx->lineNo ));

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
    char     zMark[ MAX_HEREMARK_LEN ];
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
        if (*pzScan++ == '\n')
            AG_ABEND( aprf( zErrMsg, pzProg, "HereString missing the mark",
                            pCurCtx->pzFileName, pCurCtx->lineNo ));
    }

    /*
     *  Copy the marker, noting its length
     */
    {
        char* pz = zMark;
        while (ISNAMECHAR( *pzScan )) {
            if (++markLen >= sizeof(zMark))
                AG_ABEND( aprf( zErrMsg, pzProg, "HereString mark "
                                STR( MAX_HEREMARK_LEN ) " or more chars",
                                pCurCtx->pzFileName, pCurCtx->lineNo ));

            *(pz++) = *(pzScan++);
        }
        if (markLen == 0)
            AG_ABEND( aprf( zErrMsg, pzProg, "HereString missing the mark",
                            pCurCtx->pzFileName, pCurCtx->lineNo ));
        *pz = NUL;
    }

    pzDest = pzScan;
    yylval = (YYSTYPE)pzDest;

    /*
     *  Skip forward to the EOL after the marker.
     */
    pzScan = strchr( pzScan, '\n' );
    if (pzScan == NULL)
        AG_ABEND( aprf( zErrMsg, pzProg, "Unterminated HereString",
                        pCurCtx->pzFileName, pCurCtx->lineNo ));

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
                pCurCtx->lineNo++;
                goto lineDone;

            case NUL:
                AG_ABEND( aprf( zErrMsg, pzProg, "Unterminated HereString",
                                pCurCtx->pzFileName, pCurCtx->lineNo ));
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
     *  It is a quoted string.  Process the escape sequence characters
     *  (in the set "abfnrtv") and make sure we find a closing quote.
     */
    char* pzD = pzScan;
    char* pzS = pzScan;

    yylval = (YYSTYPE)pzD;

    for (;;) {
        /*
         *  IF the next character is the quote character, THEN we may end the
         *  string.  We end it unless the next non-blank character *after* the
         *  string happens to also be a quote.  If it is, then we will change
         *  our quote character to the new quote character and continue
         *  condensing text.
         */
        while (*pzS == q) {
            *pzD = NUL;
        scan_for_quote:
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

            case '/':
                /*
                 *  Allow for a comment embedded in the concatenated string.
                 */
                switch (pzS[1]) {
                default:  return pzS;
                case '/':
                    /*
                     *  Skip to end of line
                     */
                    for (;;) {
                        char ch = *++pzS;
                        switch (ch) {
                        case '\n': goto scan_for_quote;
                        case '\0': return pzS;
                        default:   break;
                        }
                    }
                case '*':
                    /*
                     *  Skip to terminating star slash
                     */
                    pzS = strstr( pzS+2, "*/" );
                    if (pzS == NULL)
                        return (char*)zNil;
                    pzS++;
                    goto scan_for_quote;
                }

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
            return NULL;

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
                    return NULL;

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
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defLex.c */
