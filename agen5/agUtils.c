
/*
 *  agUtils.c
 *  $Id: agUtils.c,v 1.6 2000/09/28 03:51:39 bkorb Exp $
 *  This is the main routine for autogen.
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

#include <time.h>
#include <utime.h>
#include <ctype.h>

#include "autogen.h"

#ifdef MEMDEBUG
#  include "snprintfv/mem.h"
   static void finalMemCheck( void );
#endif

STATIC const char* skipQuote( const char* pzQte );


    EXPORT void
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

        case 0:
            if (! HAVE_OPT( DEFINITIONS ))
                OPT_ARG( DEFINITIONS ) = "-";
            break;
        }
    }

    /*
     *  IF the definitions file has been disabled,
     *  THEN a template *must* have been specified.
     */
    if (  (! ENABLED_OPT( DEFINITIONS ))
       && (! HAVE_OPT( OVERRIDE_TPL )) ) {
        fputs( "AutoGen error:  no template was specified\n", stderr );
        AG_ABEND;
    }

    /*
     *  IF we do not have a base-name option, then we compute some value
     */
    if (! HAVE_OPT( BASE_NAME )) do {
        char* pz;
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
        if (pz++ == (char*)NULL)
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
        pzD = AGALOC( strlen( pz )+1, "def file name" );

        while ((*pz != NUL) && (*pz != '.'))  *(pzD++) = *(pz++);
        *pzD = NUL;
    } while (AG_FALSE);

    strequate( OPT_ARG( EQUATE ));

    /*
     *  IF we have some defines to put in our environment, ...
     */
    if (HAVE_OPT( DEFINE )) {
        int        ct  = STACKCT_OPT(  DEFINE );
        char**     ppz = STACKLST_OPT( DEFINE );

        do  {
            char* pz = *(ppz++);
            /*
             *  IF there is no associated value,  THEN set it to '1'.
             *  There are weird problems with empty defines.
             */
            if (strchr( pz, '=' ) == (char*)NULL) {
                size_t siz = strlen( pz )+3;
                char*  p   = AGALOC( siz, "env define" );
                strcpy( p, pz );
                strcat( p, "=1" );
                pz = p;
            }

            /*
             *  Now put it in the environment
             */
            putenv( pz );
        } while (--ct > 0);
    }
}


    EXPORT const char*
getDefine( const char* pzDefName )
{
    char**  ppz;
    int     ct;
    if (HAVE_OPT( DEFINE )) {
        ct  = STACKCT_OPT(  DEFINE );
        ppz = STACKLST_OPT( DEFINE );

        while (ct-- > 0) {
            char* pz   = *(ppz++);
            char* pzEq = strchr( pz, '=' );
            int   res;

            if (pzEq != (char*)NULL)
                *pzEq = NUL;

            res = strcmp( pzDefName, pz );
            if (pzEq != (char*)NULL)
                *pzEq = '=';

            if (res == 0)
                return (pzEq != (char*)NULL) ? pzEq+1 : "";
        }
    }
    return getenv( pzDefName );
}


    EXPORT unsigned int
doEscapeChar( const char* pzIn, char* pRes )
{
    unsigned int  res = 1;

    switch (*pRes = *pzIn++) {
    case NUL:         /* NUL - end of input string */
    case '\n':        /* NL  - Omit newline        */
        return 0;

    case 't':
        *pRes = '\t'; /* TAB */
        break;
    case 'n':
        *pRes = '\n'; /* NEWLINE (LineFeed) */
        break;
    case 'f':
        *pRes = '\f'; /* FormFeed (NewPage) */
        break;
    case 'r':
        *pRes = '\r'; /* Carriage Return    */
        break;
    case 'v':
        *pRes = '\v'; /* Vertical Tab       */
        break;
    case 'b':
        *pRes = '\b'; /* backspace          */
        break;
    case 'a':
        *pRes = '\a'; /* Bell               */
        break;

    case 'x':         /* HEX Escape       */
        if (isxdigit( *pzIn ))  {
            unsigned int  val;
            unsigned char ch = *pzIn++;

            if ((ch >= 'A') && (ch <= 'F'))
                val = 10 + (ch - 'A');
            else if ((ch >= 'a') && (ch <= 'f'))
                val = 10 + (ch - 'a');
            else val = ch - '0';

            ch = *pzIn;

            if (! isxdigit( ch )) {
                *pRes = val;
                res   = 2;
                break;
            }
            val <<= 4;
            if ((ch >= 'A') && (ch <= 'F'))
                val += 10 + (ch - 'A');
            else if ((ch >= 'a') && (ch <= 'f'))
                val += 10 + (ch - 'a');
            else val += ch - '0';

            res = 3;
            *pRes = val;
        }
        break;

    default:
        /*
         *  IF the character copied was an octal digit,
         *  THEN set the output character to an octal value
         */
        if (isdigit( *pRes ) && (*pRes < '8'))  {
            unsigned int  val = *pRes - '0';
            unsigned char ch  = *pzIn++;

            /*
             *  IF the second character is *not* an octal digit,
             *  THEN save the value and bail
             */
            if ((ch < '0') || (ch > '7')) {
                *pRes = val;
                break;
            }

            val = (val<<3) + (ch - '0');
            ch  = *pzIn;
            res = 2;

            /*
             *  IF the THIRD character is *not* an octal digit,
             *  THEN save the value and bail
             */
            if ((ch < '0') || (ch > '7')) {
                *pRes = val;
                break;
            }

            /*
             *  IF the new value would not be too large,
             *  THEN add on the third and last character value
             */
            if ((val<<3) < 0xFF) {
                val = (val<<3) + (ch - '0');
                res = 3;
            }

            *pRes = val;
            break;
        }
    }

    return res;
}

/*
 *  The following routine scans over quoted text, shifting
 *  it in the process and eliminating the starting quote,
 *  ending quote and any embedded backslashes.  They may
 *  be used to embed the quote character in the quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
    EXPORT char*
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
                unsigned int ct = doEscapeChar( pzQte, p-1 );
                /*
                 *  IF the advance is zero,
                 *  THEN we either have end of string (caught above),
                 *       or we have an escaped new-line,
                 *       which is to be ignored.
                 *  ELSE advance the quote scanning pointer by ct
                 */
                if (ct == 0) {
                    p--;     /* move destination back one character */
                    pzQte++; /* skip over new-line character        */
                } else
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
    STATIC const char*
skipQuote( const char* pzQte )
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
                unsigned int ct = doEscapeChar( pzQte, p );
                /*
                 *  IF the advance is zero,
                 *  THEN we either have end of string (caught above),
                 *       or we have an escaped new-line,
                 *       which is to be ignored.
                 *  ELSE advance the quote scanning pointer by ct
                 */
                if (ct == 0) {
                    pzQte++; /* skip over new-line character        */
                } else
                    pzQte += ct;
            } /* if (q == '\'')      */
        }     /* switch (*pzQte++)   */
    }         /* while (*pzQte != q) */

    return pzQte+1; /* Return addr of char after the terminating quote */
}


    EXPORT const char*
skipScheme( const char* pzSrc,  const char* pzEnd )
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


    EXPORT const char*
skipExpression( const char* pzSrc, size_t len )
{
    const char* pzEnd = pzSrc + len;

 guess_again:

    while (isspace( *pzSrc )) pzSrc++;
    if (pzSrc >= pzEnd)
        return pzEnd;
    switch (*pzSrc) {
    case ';':
        pzSrc = strchr( pzSrc, '\n' );
        if (pzSrc == (const char*)NULL)
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


#ifndef MEMDEBUG


    void*
ag_alloc( size_t sz, const char* pzWhat )
{
    void* p = malloc( sz );
    if ((p == (void*)NULL) && (pzWhat != NULL)) {
        fprintf( stderr, zAllocErr, pzProg, sz, pzWhat );
        LOAD_ABORT( pCurTemplate, pCurMacro, pzWhat );
    }
    return p;
}


    void*
ag_realloc( void* p, size_t sz, const char* pzWhat )
{
    void* np = p ? realloc( p, sz ) : malloc( sz );
    if (np == (void*)NULL) {
        if (pzWhat != NULL) {
            fprintf( stderr, zAllocErr, pzProg, sz, pzWhat );
            LOAD_ABORT( pCurTemplate, pCurMacro, pzWhat );
        }
        if (p != NULL)
            free( p );
    }

    return np;
}

#else
STATIC tMemMgmt memHead = { &memHead, &memHead, (char*)NULL, "ROOT" };
#define CHECK_CT 128
#define SPARE    128


    void*
ag_alloc( size_t sz, const char* pzWhat, const char* pz )
{
    size_t    asz = sz + sizeof( tMemMgmt ) + CHECK_CT + SPARE;
    tMemMgmt* p = (tMemMgmt*)malloc( asz & ~0x3F );

    if (p == (tMemMgmt*)NULL) {
        if (pzWhat == NULL)
            return (void*)NULL;

        fprintf( stderr, zAllocErr, pzProg, sz, pzWhat );
        LOAD_ABORT( pCurTemplate, pCurMacro, pzWhat );
    }

    /*
     *  Link new entry to end of chain
     */
    p->pPrev        = memHead.pPrev;
    p->pPrev->pNext = p;
    memHead.pPrev   = p;
    p->pNext        = &memHead;

    p->pEnd = ((char*)(p+1)) + sz;
    memset( (void*)p->pEnd, '~', CHECK_CT );
    p->pzWhence = pz;

    return (void*)(p+1);
}


    void*
ag_realloc( void* p, size_t sz, const char* pzWhat, const char* pz )
{
    size_t      asz = sz + sizeof( tMemMgmt ) + CHECK_CT + SPARE;
    tMemMgmt*   np  = ((tMemMgmt*)p)-1;
    tMemMgmt    sv  = *np;

    checkMem( np );
    np = (tMemMgmt*)(p ? realloc( (void*)np, asz & ~0x3F )
                       : malloc( asz & ~0x3F ));

    if (np == (tMemMgmt*)NULL) {
        if (pzWhat == NULL) {
            if (p != NULL)
                free( (void*)p );

            /*
             *  Unlink old entry
             */
            sv.pPrev->pNext = sv.pNext;
            sv.pNext->pPrev = sv.pPrev;
            return (void*)NULL;
        }

        fprintf( stderr, zAllocErr, pzProg, sz, pzWhat );
        LOAD_ABORT( pCurTemplate, pCurMacro, pzWhat );
    }

    /*
     *  Link other entries to new allocation
     */
    np->pPrev->pNext = np;
    np->pNext->pPrev = np;

    np->pEnd = ((char*)(np+1)) + sz;
    memset( (void*)np->pEnd, '~', CHECK_CT );
    np->pzWhence = pz;

    return (void*)(np+1);
}


    STATIC void
checkMem( tMemMgmt* pMM )
{
    char* p  = pMM->pEnd;
    int   ct = CHECK_CT;

    do  {
        if (*(p++) != '~') {
            fprintf( stderr, "Stray pointer %d bytes after %d-byte %s end\n",
                     CHECK_CT - ct, pMM->pEnd - (char*)(pMM+1),
                     pMM->pzWhence );
            fclose( stderr );
            fclose( stdout );
            p = (char*)NULL;
            *p = '~'; /* PAGE FAULT */
            _exit( EXIT_FAILURE );
        }
    } while (--ct > 0);
}


    void
ag_free( void* p )
{
    tMemMgmt*   np  = ((tMemMgmt*)p)-1;
    checkMem( np );

    /*
     *  Unlink old entry
     */
    np->pPrev->pNext = np->pNext;
    np->pNext->pPrev = np->pPrev;
    free( (void*)np );
}


    static void
finalMemCheck( void )
{
    tMemMgmt*  pMM = memHead.pNext;

    fputs( "\nResidual allocation list:\n", stderr );
    while (pMM != &memHead) {
        checkMem( pMM );
        fprintf( stderr, "%12d bytes from %s\n",
                 pMM->pEnd - (char*)(pMM+1), pMM->pzWhence );
        pMM = pMM->pNext;
    }
}


    char*
dupString( const char* pz, const char* pzDupFrom )
{
    char*   pzRes;
    size_t  len = strlen( pz )+1;

    /*
     *  There are some systems out there where autogen is
     *  broken if "strdup" is allowed to duplicate strings
     *  smaller than 32 bytes.  This ensures that we work.
     *  We also round up everything up to 32 bytes.
     */
    if (len < 0x20)
        len = 0x20;
    else len = (len + 0x20) & ~0x1F;

    pzRes = ag_alloc( len, pzDupFrom );

    if (pzRes == (char*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 len, "strdup" );
        AG_ABEND;
    }

    strcpy( pzRes, pz );
    return pzRes;
}
#endif
/* end of agUtils.c */
