
/*
 *  agUtils.c
 *  $Id: agUtils.c,v 4.16 2007/10/07 16:54:54 bkorb Exp $
 *
 *  Time-stamp:        "2007-07-04 11:14:34 bkorb"
 *  Last Committed:    $Date: 2007/10/07 16:54:54 $
 *
 *  This is the main routine for autogen.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2007 by Bruce Korb - all rights reserved
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

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by mk-fwd */
static tCC*
skipQuote( tCC* pzQte );
/* = = = END-STATIC-FORWARD = = = */

#ifndef HAVE_STRLCPY
size_t
strlcpy( char* dest, tCC* src, size_t n )
{
    char* pz = dest;
    tCC* ps = src;
    size_t sz = 0;

    for (;;) {
        if ((*(pz++) = *(src++)) == NUL)
            break;

        /*
         *  This is the unusual condition.  Do the exceptional work here.
         */
        if (--n <= 0) {
            pz[-1] = NUL;
            sz = strlen( src ) + 1; /* count of chars not copied out */
            break;
        }
    }

    assert( sz + (src - ps) == strlen( ps ) + 1 );

    return sz + (src - ps);
}
#endif


LOCAL char*
aprf( char const* pzFmt, ... )
{
    char* pz;
    va_list ap;
    va_start( ap, pzFmt );
    (void)vasprintf( &pz, pzFmt, ap );
    va_end( ap );

    if (pz == NULL) {
        tSCC zMsg[] = "could not allocate for or formatting failed on:\n";
        char z[ 256 ];
        strcpy( z, zMsg );
        strncpy( z + sizeof( zMsg )-1, pzFmt, sizeof(z) - sizeof(zMsg));
        z[ sizeof(z)-1 ] = NUL;
        AG_ABEND( z );
    }
    return pz;
}


LOCAL char*
mkstempPat( void )
{
    static char const * pzTmpDir = NULL;

    if (pzTmpDir == NULL) {
        static char const * const envlist[] = {
            "TMPDIR", "TEMP", "TMP", NULL };
        char const * const * penv = envlist;
        for (;;) {
            pzTmpDir = getenv(*penv);
            if (pzTmpDir != NULL)
                break;
            if (*(++penv) == NULL) {
                pzTmpDir = "/tmp";
                break;
            }
        }
    }

    {
        char* pzRes = aprf("%s/agtmp.XXXXXX", pzTmpDir);
        manageAllocatedData((void*)pzRes);
        TAGMEM( pzRes, "temp file template" );
        return pzRes;
    }
}


LOCAL void
doOptions( int arg_ct, char** arg_vec )
{
    /*
     *  Advance the argument counters and pointers past any
     *  command line options
     */
    {
        tSCC zOnlyOneSrc[] = "%s ERROR:  Too many definition files\n";
        int  optCt = optionProcess( &autogenOptions, arg_ct, arg_vec );

        /*
         *  Make sure we have a source file, even if it is "-" (stdin)
         */
        switch (arg_ct - optCt) {
        case 1:
            if (! HAVE_OPT( DEFINITIONS )) {
                OPT_ARG( DEFINITIONS ) = *(arg_vec + optCt);
                break;
            }
            /* FALLTHROUGH */

        default:
            fprintf( stderr, zOnlyOneSrc, pzProg );
            USAGE( EXIT_FAILURE );
            /* NOTREACHED */

        case 0:
            if (! HAVE_OPT( DEFINITIONS ))
                OPT_ARG( DEFINITIONS ) = "-";
            break;
        }
    }

    if (! HAVE_OPT( TIMEOUT ))
        OPT_ARG(TIMEOUT) = (char const *)AG_DEFAULT_TIMEOUT;

    /*
     *  IF the definitions file has been disabled,
     *  THEN a template *must* have been specified.
     */
    if (  (! ENABLED_OPT( DEFINITIONS ))
       && (! HAVE_OPT( OVERRIDE_TPL )) )
        AG_ABEND( "no template was specified" );

    /*
     *  IF we do not have a base-name option, then we compute some value
     */
    if (! HAVE_OPT( BASE_NAME )) do {
        tCC*  pz;
        char* pzD;

        if (! ENABLED_OPT( DEFINITIONS )) {
            OPT_ARG( BASE_NAME ) = "baseless";
            break;
        }

        pz = strrchr( OPT_ARG( DEFINITIONS ), '/' );
        /*
         *  Point to the character after the last '/', or to the full
         *  definition file name, if there is no '/'.
         */
        if (pz++ == NULL)
            pz = OPT_ARG( DEFINITIONS );

        /*
         *  IF input is from stdin, then use "stdin"
         */
        if ((pz[0] == '-') && (pz[1] == NUL)) {
            OPT_ARG( BASE_NAME ) = "stdin";
            break;
        }

        /*
         *  Otherwise, use the basename of the definitions file
         */
        OPT_ARG( BASE_NAME ) = \
        pzD = AGALOC( strlen( pz )+1, "derived base name" );

        while ((*pz != NUL) && (*pz != '.'))  *(pzD++) = *(pz++);
        *pzD = NUL;
    } while (AG_FALSE);

    strequate( OPT_ARG( EQUATE ));

    /*
     *  IF we have some defines to put in our environment, ...
     */
    if (HAVE_OPT( DEFINE )) {
        int     ct  = STACKCT_OPT(  DEFINE );
        tCC**   ppz = STACKLST_OPT( DEFINE );

        do  {
            tCC* pz = *(ppz++);
            /*
             *  IF there is no associated value,  THEN set it to '1'.
             *  There are weird problems with empty defines.
             *  FIXME:  we loose track of this memory.  Don't know what to do,
             *  really, there is no good recovery mechanism for environment
             *  data.
             */
            if (strchr( pz, '=' ) == NULL) {
                size_t siz = strlen( pz )+3;
                char*  p   = AGALOC( siz, "env define" );

                strcpy( p, pz );
                strcpy( p+siz-3, "=1" );
                pz = p;
            }

            /*
             *  Now put it in the environment
             */
            putenv( (char*)pz );
        } while (--ct > 0);
    }
}


LOCAL tCC*
getDefine( tCC* pzDefName, ag_bool check_env )
{
    tCC**   ppz;
    int     ct;
    if (HAVE_OPT( DEFINE )) {
        ct  = STACKCT_OPT(  DEFINE );
        ppz = STACKLST_OPT( DEFINE );

        while (ct-- > 0) {
            tCC*  pz   = *(ppz++);
            char* pzEq = strchr( pz, '=' );
            int   res;

            if (pzEq != NULL)
                *pzEq = NUL;

            res = strcmp( pzDefName, pz );
            if (pzEq != NULL)
                *pzEq = '=';

            if (res == 0)
                return (pzEq != NULL) ? pzEq+1 : zNil;
        }
    }
    return check_env ? getenv( pzDefName ) : NULL;
}


/*
 *  The following routine scans over quoted text, shifting
 *  it in the process and eliminating the starting quote,
 *  ending quote and any embedded backslashes.  They may
 *  be used to embed the quote character in the quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
LOCAL char*
spanQuote( char* pzQte )
{
    char  q = *pzQte;          /*  Save the quote character type */
    char* p = pzQte++;         /*  Destination pointer           */

    while (*pzQte != q) {
        switch (*p++ = *pzQte++) {
        case NUL:
            return pzQte-1;      /* Return address of terminating NUL */

        case '\\':
            if (q != '\'') {
                int ct = ao_string_cook_escape_char(pzQte, p-1, 0x7F);
                if (p[-1] == 0x7F)  p--;
                pzQte += ct;

            } else {
                switch (*pzQte) {
                case '\\':
                case '\'':
                case '#':
                    p[-1] = *pzQte++;
                }
            }
            break;

        default:
            ;
        }
    }

    *p = NUL;
    return pzQte+1; /* Return addr of char after the terminating quote */
}

/*
 *  The following routine skips over quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
static tCC*
skipQuote( tCC* pzQte )
{
    char  q = *pzQte++;        /*  Save the quote character type */

    while (*pzQte != q) {
        switch (*pzQte++) {
        case NUL:
            return pzQte-1;      /* Return address of terminating NUL */

        case '\\':
            if (q == '\'') {
                /*
                 *  Single quoted strings process the backquote specially
                 *  only in fron of these three characters:
                 */
                switch (*pzQte) {
                case '\\':
                case '\'':
                case '#':
                    pzQte++;
                }

            } else {
                char p[10];  /* provide a scratch pad for escape processing */
                int ct = ao_string_cook_escape_char( pzQte, p, 0x7F );
                pzQte += ct;
            } /* if (q == '\'')      */
        }     /* switch (*pzQte++)   */
    }         /* while (*pzQte != q) */

    return pzQte+1; /* Return addr of char after the terminating quote */
}


LOCAL tCC*
skipScheme( tCC* pzSrc,  tCC* pzEnd )
{
    int  level = 0;

    for (;;) {
        if (pzSrc >= pzEnd)
            return pzEnd;
        switch (*(pzSrc++)) {
        case '(':
            level++;
            break;

        case ')':
            if (--level == 0)
                return pzSrc;
            break;

        case '"':
            pzSrc = skipQuote( pzSrc-1 );
        }
    }
}


LOCAL tCC*
skipExpression( tCC* pzSrc, size_t len )
{
    tCC* pzEnd = pzSrc + len;

 guess_again:

    while (isspace( *pzSrc )) pzSrc++;
    if (pzSrc >= pzEnd)
        return pzEnd;
    switch (*pzSrc) {
    case ';':
        pzSrc = strchr( pzSrc, '\n' );
        if (pzSrc == NULL)
            return pzEnd;
        goto guess_again;

    case '(':
        return skipScheme( pzSrc, pzEnd );

    case '"':
    case '\'':
    case '`':
        pzSrc = skipQuote( pzSrc );
        return (pzSrc > pzEnd) ? pzEnd : pzSrc;

    default:
        break;
    }

    while (! isspace( *pzSrc ))  pzSrc++;
    return (pzSrc > pzEnd) ? pzEnd : pzSrc;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agUtils.c */
