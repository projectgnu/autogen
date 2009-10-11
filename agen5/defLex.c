
/*
 *  $Id$
 *
 *  Time-stamp:        "2008-07-26 10:06:56 bkorb"
 *
 *  This module scans the template variable declarations and passes
 *  tokens back to the parser.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
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
te_dp_event aKeywordTkn[] = { KEYWORD_TABLE };
#undef _KW_

#define KEYWORD_CT  (sizeof( apzKeywords ) / sizeof( apzKeywords[0] ))

#define ERROR  (-1)
#define FINISH (-1)

#define SET_LIT_TKN(t) lastToken = DP_EV_LIT_ ## t; *(pCurCtx->pzScan++) = NUL;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by mk-fwd */
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
    if (IS_WHITESPACE_CHAR( *pCurCtx->pzScan )) {
        char* pz = pCurCtx->pzScan;
        if (*pz == '\n')
            pCurCtx->lineNo++;
        *(pz++) = NUL;

        /*
         *  This ensures that any names found previously
         *  are NUL terminated.
         */
        while (IS_WHITESPACE_CHAR(*pz)) {
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

        pz = assembleHereString(pCurCtx->pzScan + 2);
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
        if (strncmp( pCurCtx->pzScan+1, "'(", (size_t)2) == 0) {
            alist_to_autogen_def();
            goto scanAgain;

        } else {
            char* pz = strchr( pCurCtx->pzScan, ';' );

            for (;;) {
                if (pz == NULL) {
                    pz = pCurCtx->pzScan + strlen( pCurCtx->pzScan );
                    break;
                }
                if (IS_WHITESPACE_CHAR( pz[1] )) {
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
        int line_no = pCurCtx->lineNo;
        char* pz = ao_string_cook( pCurCtx->pzScan, &line_no);

        if (pz == NULL)
            goto NUL_error;

        pz_token = pCurCtx->pzScan;

        pCurCtx->pzScan = pz;

        lastToken = DP_EV_STRING;
        pz = runShell( (char const*)pz_token );
        pCurCtx->lineNo = line_no;

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
    /*
     *  First time through, return the DP_EV_END token.
     *  Second time through, we really finish.
     */
    if (pCurCtx->pzScan == zNil) {
        pCurCtx->pCtx = pDoneCtx;
        pDoneCtx      = pCurCtx;

        return DP_EV_INVALID;
    }

    pCurCtx->pzScan = (char*)zNil;
    return DP_EV_END;
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
    int      next_ln;
    SCM      res;

    /*
     *  NUL terminate the Scheme expression, run it, then restore
     *  the NUL-ed character.
     */
    if (*pzEnd == NUL)
        AG_ABEND(aprf(zErrMsg, pzProg,
                      "end of Guile/scheme expression not found",
                      pCurCtx->pzCtxFname, pCurCtx->lineNo));

    *pzEnd  = NUL;
    next_ln = pCurCtx->lineNo + count_nl(pzText);

    procState = PROC_STATE_GUILE_PRELOAD;
    res = ag_scm_c_eval_string_from_file_line(
        pzText, pCurCtx->pzCtxFname, pCurCtx->lineNo );
    procState = PROC_STATE_LOAD_DEFS;
    *pzEnd = endCh;

    pCurCtx->pzScan = pzEnd;
    pzEnd = (char*)resolveSCM( res ); /* ignore const-ness */
    pCurCtx->lineNo = next_ln;

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
    size_t res_len;
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
     *  Run the scheme expression.  The result is autogen definition text.
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
    AGFREE(pzText);

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
    if (  IS_DEC_DIGIT_CHAR( *pzScan )
       || (  (*pzScan == '-')
          && IS_DEC_DIGIT_CHAR( pzScan[1] )
       )  )  {
        pz_token = pzScan;
        (void)strtol( pzScan, &pzScan, 0 );
        *pRetVal = DP_EV_NUMBER;
        return pzScan;
    }

    if (! IS_UNQUOTABLE_CHAR(*pzScan))
        AG_ABEND( aprf("%s Error: Invalid input char '%c' in %s on line %d\n",
                       pzProg, *pzScan, pCurCtx->pzCtxFname, pCurCtx->lineNo));

    {
        unsigned char* pz = (unsigned char*)pzScan;

        while (IS_VALUE_NAME_CHAR(*pz))          pz++;

        if (IS_UNQUOTABLE_CHAR(*pz)) {
            *pRetVal = DP_EV_OTHER_NAME;
            while (IS_UNQUOTABLE_CHAR(*++pz))    ;
        } else
            *pRetVal = DP_EV_VAR_NAME;

        /*
         *  Return a NAME token, maybe.
         *  If the name is actually a keyword,
         *  we will return that token code instead.
         */
        pz_token = pzScan;
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
static char*
assembleHereString( char* pzScan )
{
    static char const endless[] = "Unterminated HereString";
    ag_bool  trimTabs = AG_FALSE;
    char     zMark[ MAX_HEREMARK_LEN ];
    size_t   markLen = 0;
    char*    pzDest;
    int      here_string_line_no;

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
    while (IS_WHITESPACE_CHAR( *pzScan )) {
        if (*pzScan++ == '\n')
            AG_ABEND( aprf( zErrMsg, pzProg, "HereString missing the mark",
                            pCurCtx->pzCtxFname, pCurCtx->lineNo ));
    }

    /*
     *  Copy the marker, noting its length
     */
    {
        char* pz = zMark;
        while (IS_VARIABLE_NAME_CHAR( *pzScan )) {
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
        AG_ABEND( aprf( zErrMsg, pzProg, endless, pCurCtx->pzCtxFname,
                        pCurCtx->lineNo ));

    /*
     *  And skip the first new line + conditionally skip tabs
     */
    here_string_line_no = pCurCtx->lineNo++;
    pzScan++;

    if (trimTabs)
        while (*pzScan == '\t')  ++pzScan;

    /*
     *  FOR as long as the text does not match the mark
     *       OR it matches but is a substring
     *  DO copy characters
     */
    while (  (strncmp( pzScan, zMark, markLen ) != 0)
          || IS_VARIABLE_NAME_CHAR( pzScan[ markLen ]) )  {

        for (;;) {
            switch (*(pzDest++) = *(pzScan++)) {
            case '\n':
                pCurCtx->lineNo++;
                goto lineDone;

            case NUL:
                AG_ABEND( aprf( zErrMsg, pzProg, endless, pCurCtx->pzCtxFname,
                                here_string_line_no));
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
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defLex.c */
