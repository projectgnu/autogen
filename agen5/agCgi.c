
/*
 *  agCgi.c
 *  $Id$
 *
 *  Time-stamp:        "2009-07-31 11:01:07 bkorb"
 *
 *  This is a CGI wrapper for AutoGen.  It will take POST-method
 *  name-value pairs and emit AutoGen definitions to a spawned
 *  AutoGen process.
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

typedef struct {
    char const*  pzName;
    char*        pzValue;
} tNameMap;

#define ENV_TABLE \
    _ET_( SERVER_SOFTWARE ) \
    _ET_( SERVER_NAME ) \
    _ET_( GATEWAY_INTERFACE ) \
    _ET_( SERVER_PROTOCOL ) \
    _ET_( SERVER_PORT ) \
    _ET_( REQUEST_METHOD ) \
    _ET_( PATH_INFO ) \
    _ET_( PATH_TRANSLATED ) \
    _ET_( SCRIPT_NAME ) \
    _ET_( QUERY_STRING ) \
    _ET_( REMOTE_HOST ) \
    _ET_( REMOTE_ADDR ) \
    _ET_( AUTH_TYPE ) \
    _ET_( REMOTE_USER ) \
    _ET_( REMOTE_IDENT ) \
    _ET_( CONTENT_TYPE ) \
    _ET_( CONTENT_LENGTH ) \
    _ET_( HTTP_ACCEPT ) \
    _ET_( HTTP_USER_AGENT ) \
    _ET_( HTTP_REFERER )

static tNameMap nameValueMap[] = {
#define _ET_(n) { #n, NULL },
    ENV_TABLE
#undef _ET_
    { NULL, NULL }
};

typedef enum {
#define _ET_(n) n ## _IDX,
    ENV_TABLE
#undef _ET_
    NAME_CT
} tNameIdx;

#define pzCgiMethod nameValueMap[ REQUEST_METHOD_IDX ].pzValue
#define pzCgiQuery  nameValueMap[ QUERY_STRING_IDX   ].pzValue
#define pzCgiLength nameValueMap[ CONTENT_LENGTH_IDX ].pzValue

static char const zOops[] =
"Content-type: text/plain\n\n"
"AutoGen form processing error:\n";

/* = = = START-STATIC-FORWARD = = = */
static char*
parseInput( char* pzSrc, int len );
/* = = = END-STATIC-FORWARD = = = */

LOCAL void
loadCgi( void )
{
    /*
     *  Redirect stderr to a file.  If it gets used, we must trap it
     *  and emit the content-type: preamble before actually emitting it.
     *  First, tho, do a simple stderr->stdout redirection just in case
     *  we stumble before we're done with this.
     */
    dup2( STDOUT_FILENO, STDERR_FILENO );
    (void)fdopen( STDERR_FILENO, "w" FOPEN_BINARY_FLAG );
    pzOopsPrefix = zOops;
    {
        int tmpfd;
        pzTmpStderr = mkstempPat();
        tmpfd = mkstemp( pzTmpStderr );
        if (tmpfd < 0)
            AG_ABEND(aprf("failed to create temp file from `%s'", pzTmpStderr));
        dup2( tmpfd, STDERR_FILENO );
        close( tmpfd );
    }

    /*
     *  Pull the CGI-relevant environment variables.  Anything we don't find
     *  gets an empty string default.
     */
    {
        tNameMap* pNM = nameValueMap;
        tNameIdx  ix  = (tNameIdx)0;

        do  {
            pNM->pzValue = getenv( pNM->pzName );
            if (pNM->pzValue == NULL)
                pNM->pzValue = (char*)zNil;
        } while (pNM++, ++ix < NAME_CT);
    }

    pBaseCtx = (tScanCtx*)AGALOC( sizeof( tScanCtx ), "CGI context" );
    memset( (void*)pBaseCtx, 0, sizeof( tScanCtx ));

    {
        size_t textLen = strtoul( pzCgiLength, NULL, 0 );
        char*  pzText;

        if (strcasecmp( pzCgiMethod, "POST" ) == 0) {
            if (textLen == 0)
                AG_ABEND( "No CGI data were received" );

            pzText  = AGALOC( textLen + 1, "CGI POST text" );
            if (fread(pzText, (size_t)1, textLen, stdin) != textLen)
                AG_ABEND( aprf( zCannot, errno, "read", "CGI text",
                                strerror( errno )));

            pzText[ textLen ] = NUL;

            pBaseCtx->pzData = parseInput( pzText, (int)textLen );
            AGFREE( pzText );

        } else if (strcasecmp( pzCgiMethod, "GET" ) == 0) {
            if (textLen == 0)
                textLen = strlen( pzCgiQuery );
            pBaseCtx->pzData = parseInput( pzCgiQuery, (int)textLen );

        } else {
            AG_ABEND( aprf("invalid CGI request method: ``%s''", pzCgiMethod));
            /* NOTREACHED */
#ifdef  WARNING_WATCH
            pzText = NULL;
#endif
        }
    }

    pBaseCtx->lineNo     = 1;
    pBaseCtx->pzCtxFname = "@@ CGI Definitions @@";
    pBaseCtx->pzScan     = pBaseCtx->pzData;
}


static char*
parseInput( char* pzSrc, int len )
{
    tSCC   zDef[] = "Autogen Definitions cgi;\n";
#   define defLen   (sizeof( zDef ) - 1)
    char*  pzRes  = AGALOC( (len * 2) + defLen + 1, "CGI Definitions" );

    memcpy( pzRes, zDef, defLen );
    (void)cgi_run_fsm( pzSrc, len, pzRes + defLen, len*2 );
    return AGREALOC( pzRes, strlen( pzRes )+1, "CGI input" );
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agCgi.c */
