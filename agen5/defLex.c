
/*
 *  $Id: defLex.c,v 4.12 2006/01/24 23:19:11 bkorb Exp $
 *  This module scans the template variable declarations and passes
 *  tokens back to the parser.
 */

/*
 *  AutoGen copyright 1992-2005 Bruce Korb
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
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */
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

#define _KW_(w) DP_EV_ ## w,
int    aKeywordTkn[] = { KEYWORD_TABLE };
#undef _KW_

#define KEYWORD_CT  (sizeof( apzKeywords ) / sizeof( apzKeywords[0] ))

#define ERROR  (-1)
#define FINISH (-1)

#define SET_LIT_TKN(t) lastToken = DP_EV_LIT_ ## t; *(pCurCtx->pzScan++) = NUL;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static void
loadScheme( void );

static void
alist_to_autogen_def( void );

static char*
assembleName( char* pzScan, te_dp_event* pRetVal );

static char*
assembleHereString( char* pzScan );
/* = = = END-STATIC-FORWARD = = = */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   LEXICAL SCANNER
 */
LOCAL te_dp_event
yylex( void )
{
    lastToken = DP_EV_INVALID;

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

    case '{': SET_LIT_TKN( O_BRACE );   break;
    case '=': SET_LIT_TKN( EQ );        break;
    case '}': SET_LIT_TKN( C_BRACE );   break;
    case '[': SET_LIT_TKN( OPEN_BKT );  break;
    case ']': SET_LIT_TKN( CLOSE_BKT ); break;
    case ';': SET_LIT_TKN( SEMI );      break;
    case ',': SET_LIT_TKN( COMMA );     break;

    case '\'':
    case '"':
    {
        char* pz = ao_string_cook( pCurCtx->pzScan, &(pCurCtx->lineNo));
        if (pz == NULL)
            goto NUL_error;

        pz_token = pCurCtx->pzScan;

        lastToken = DP_EV_STRING;
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
            lastToken = DP_EV_INVALID;
            return DP_EV_INVALID;
        }

        lastToken = DP_EV_HERE_STRING;
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

            lastToken = DP_EV_STRING;
            pz_token = pz;
            break;
        }

    case '`':
    {
        char* pz = ao_string_cook( pCurCtx->pzScan, &(pCurCtx->lineNo));

        if (pz == NULL)
            goto NUL_error;

        pz_token = pCurCtx->pzScan;

        pCurCtx->pzScan = pz;

        if (pzShellProgram == NULL)
            pzShellProgram = getDefine( zShellEnv, AG_TRUE );

        lastToken = DP_EV_STRING;
        pz = runShell( (const char*)pz_token );
        if (pz == NULL)
            goto scanAgain;
        TAGMEM( pz, "shell definition string" );
        pz_token = pz;
        manageAllocatedData( pz );
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
                    pCurCtx->pzCtxFname, pCurCtx->lineNo ));
    return DP_EV_INVALID;

lex_done:
    {
        tSCC zDone[] = "";

        /*
         *  First time through, return the DP_EV_END token.
         *  Second time through, we really finish.
         */
        if (pCurCtx->pzScan == zDone) {
            pCurCtx->pCtx = pDoneCtx;
            pDoneCtx      = pCurCtx;

            return DP_EV_INVALID;
        }

        pCurCtx->pzScan = (char*)zDone;
        return DP_EV_END;
    }
}


LOCAL void
yyerror( char* s )
{
    tSCC zErrTkn[] = "%s:  ``%s''\n";
    tSCC zDf[] = "`%s'\n";

    char* pz;

    if (strlen( pCurCtx->pzScan ) > 64 )
        pCurCtx->pzScan[64] = NUL;

    switch (lastToken) {
    case DP_EV_VAR_NAME:
    case DP_EV_OTHER_NAME:
    case DP_EV_STRING:
    case DP_EV_NUMBER:
        if (strlen( pz_token ) > 64 )
            pz_token[64] = NUL;

        pz = aprf( zErrTkn, DP_EVT_NAME( lastToken ), pz_token );
        break;

    default:
        pz = aprf( zDf, DP_EVT_NAME( lastToken ));
    }
    AG_ABEND( aprf( "%s:  in %s on line %d\n"
                    "\ttoken in error:  %s\n"
                    "\t[[...<error-text>]] %s\n\n"
                    "Likely causes:  a mismatched quote, a value that needs "
                    "quoting,\n\t\tor a missing semi-colon\n",
                    s, pCurCtx->pzCtxFname, pCurCtx->lineNo, pz,
                    pCurCtx->pzScan ));
}


static void
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
        pzText, pCurCtx->pzCtxFname, pCurCtx->lineNo );
    procState = PROC_STATE_LOAD_DEFS;
    *pzEnd = endCh;

    pCurCtx->pzScan = pzEnd;
    pzEnd = resolveSCM( res );
    if (strlen( pzEnd ) >= schemeLen) {
        AGDUPSTR( pzEnd, pzEnd, "SCM Result" );

        pz_token = pzEnd;
        manageAllocatedData( pz_token );
    }

    else {
        /*
         *  We know the result is smaller than the source.  Copy in place.
         */
        strcpy( pzText, pzEnd );
        pz_token = pzText;
    }

    lastToken = DP_EV_STRING;
}

/*
 *  process a single scheme expression, yielding text that gets processed
 *  into AutoGen definitions.
 */
static void
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
        pzText, pCurCtx->pzCtxFname, pCurCtx->lineNo );

    /*
     *  The result *must* be a string, or we choke.
     */
    if (! AG_SCM_STRING_P( res )) {
        tSCC zEr[] = "Scheme definition expression does not yield string:\n";
        AG_ABEND( zEr );
    }

    res_len   = AG_SCM_STRLEN( res );
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
    AGDUPSTR( pCtx->pzCtxFname, zSchemeText, "scheme text" );
    pCtx->pzScan = \
    pCtx->pzData = (char*)(pCtx+1);
    pCtx->lineNo = 0;
    memcpy( (void*)(pCtx->pzScan), (void*)AG_SCM_CHARS( res ), res_len );
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
static char*
assembleName( char* pzScan, te_dp_event* pRetVal )
{
    /*
     *  Check for a number.
     *  Scan it in and advance "pzScan".
     */
    if (  isdigit( *pzScan )
       || (  (*pzScan == '-')
          && isdigit( pzScan[1] )
       )  )  {
        pz_token = pzScan;
        (void)strtol( pzScan, &pzScan, 0 );
        *pRetVal = DP_EV_NUMBER;
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
                                pCurCtx->pzCtxFname, pCurCtx->lineNo ));

            *pRetVal = DP_EV_VAR_NAME;
        } else {
            *pRetVal = DP_EV_OTHER_NAME;
            while (zNameChars[ *pz ] != 0) pz++;
        }

        /*
         *  Return a NAME token, maybe.
         *  If the name is actually a keyword,
         *  we will return that token code instead.
         */
        pz_token   = pzScan;
        pzScan   = (char*)pz;
    }

    /*
     *  Now scan the keyword table.
     */
    if (*pRetVal == DP_EV_VAR_NAME) {
        char sv_ch = *pzScan;  /* preserve the following character */
        int  kw_ix = 0;
        *pzScan = NUL;         /* NUL terminate the name           */

        do  {
            if (streqvcmp( apzKeywords[ kw_ix ], (char*)pz_token ) == 0) {
                /*
                 *  Return the keyword token code instead of DP_EV_NAME
                 */
                *pRetVal = (te_dp_event)aKeywordTkn[ kw_ix ];
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
static char*
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
                            pCurCtx->pzCtxFname, pCurCtx->lineNo ));
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
                                pCurCtx->pzCtxFname, pCurCtx->lineNo ));

            *(pz++) = *(pzScan++);
        }
        if (markLen == 0)
            AG_ABEND( aprf( zErrMsg, pzProg, "HereString missing the mark",
                            pCurCtx->pzCtxFname, pCurCtx->lineNo ));
        *pz = NUL;
    }

    pzDest = pzScan;
    pz_token = pzDest;

    /*
     *  Skip forward to the EOL after the marker.
     */
    pzScan = strchr( pzScan, '\n' );
    if (pzScan == NULL)
        AG_ABEND( aprf( zErrMsg, pzProg, "Unterminated HereString",
                        pCurCtx->pzCtxFname, pCurCtx->lineNo ));

    /*
     *  And skip the first new line + conditionally skip tabs
     */
    pCurCtx->lineNo++;
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
                                pCurCtx->pzCtxFname, pCurCtx->lineNo ));
            }
        } lineDone:;

        if (trimTabs)
            while (*pzScan == '\t')  ++pzScan;
    } /* while strncmp ... */

    /*
     *  pzDest may still equal pz_token, if no data were copied
     */
    if (pzDest > (char*)pz_token)
         pzDest[-1] = NUL;
    else pzDest[0]  = NUL;

    return pzScan + markLen;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defLex.c */
