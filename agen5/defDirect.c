
/*
 *  defDirect.c
 *  $Id: defDirect.c,v 3.14 2003/04/19 02:40:33 bkorb Exp $
 *  This module processes definition file directives.
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
#include "autogen.h"
#include "directive.h"


tSCC zNoEndif[]   = "Definition error:  in %s line %d, #endif not found\n";
tSCC zNoMatch[]   = "Definition error:  in %s line %d, "
                    "#%s no matching start/if directive\n";
tSCC zCheckList[] = "\n#";

STATIC int  ifdefLevel = 0;
STATIC teDirectives findDirective( char* pzDirName );
STATIC teDirectives findDirective( char* pzDirName );
STATIC char* skipToEndif( char* pzScan );
STATIC char* skipToElseEnd( char* pzScan );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  processDirective
 *
 *  THIS IS THE ONLY EXTERNAL ENTRY POINT
 *
 *  A directive character has been found.
 *  Decide what to do and return a pointer to the character
 *  where scanning is to resume.
 */
EXPORT char*
processDirective( char* pzScan )
{
    const tDirTable* pTbl  = dirTable;
    char*      pzDir;
    char*      pzEnd;

    /*
     *  Search for the end of the #-directive.
     *  Replace "\\\n" sequences with "  ".
     */
    for (;;) {
        pzEnd = strchr( pzScan, '\n' );

        if (pzEnd == NULL) {
            /*
             *  The end of the directive is the end of the string
             */
            pzEnd = pzScan + strlen( pzScan );
            break;
        }
        pCurCtx->lineNo++;

        if (pzEnd[-1] != '\\') {
            /*
             *  The end of the directive is the end of the line
             *  and the line has not been continued.
             */
            *(pzEnd++) = NUL;
            break;
        }

        /*
         *  Replace the escape-newline pair with spaces and
         *  find the next end of line
         */
        pzEnd[-1] = pzEnd[0] = ' ';
    }

    /*
     *  Ignore ``#!'' as a comment, enabling a definition file to behave
     *  as a script that gets interpreted by autogen.  :-)
     */
    if (*pzScan == '!')
        return pzEnd;

    /*
     *  Find the start of the directive name
     */
    while (isspace(*pzScan)) pzScan++;
    pzDir = pzScan;

    /*
     *  Find the *END* of the directive name
     */
    while (ISNAMECHAR( *pzScan )) pzScan++;

    /*
     *  IF there is anything that follows the name, ...
     */
    if (*pzScan != NUL) {
        /*
         *  IF something funny immediately follows the directive name,
         *  THEN we will ignore it completely.
         */
        if (! isspace( *pzScan ))
            return pzEnd;

        /*
         *  Terminate the name being defined
         *  and find the start of anything else.
         */
        *pzScan++ = NUL;
        while (isspace(*pzScan)) pzScan++;
    }

    /*
     *  Trim off trailing white space
     */
    {
        char* pz = pzScan + strlen( pzScan );
        while ((pz > pzScan) && isspace( pz[-1] )) pz--;
        *pz = NUL;
    }

    pTbl = dirTable + (int)findDirective( pzDir );
    return (*(pTbl->pDirProc))( pzScan, pzEnd );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Figure out the index of a directive.  We will return the directive
 *  count if it is a bogus name.
 */

STATIC teDirectives
findDirective( char* pzDirName )
{
    teDirectives res = (teDirectives)0;
    const tDirTable*  pTbl = dirTable;

    do  {
        if (  (strneqvcmp( pzDirName, pTbl[res].pzDirName,
                           pTbl[res].nameSize ) == 0)
           && (  isspace( pzDirName[ pTbl[res].nameSize ])
              || (pzDirName[ pTbl[res].nameSize ] == NUL) )  )
            return res;

    } while (++res < DIRECTIVE_CT);

    {
        char ch;
        if (strlen( pzDirName ) > 32) {
            ch = pzDirName[32];
            pzDirName[32] = NUL;
        } else {
            ch = NUL;
        }

        fprintf( pfTrace, "WARNING:  in %s on line %d unknown directive:\n"
                 "\t#%s\n", pCurCtx->pzFileName, pCurCtx->lineNo, pzDirName );

        if (ch != NUL)
            pzDirName[32] = ch;
    }
    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Support routines for the directives
 *
 *  skipToEndif
 *
 *  Skip through the text to a matching "#endif".  We do this when we
 *  have processed the allowable text (found an "#else" after
 *  accepting the preceeding text) or when encountering a "#if*def"
 *  while skipping a block of text due to a failed test.
 */
STATIC char*
skipToEndif( char* pzScan )
{
    for (;;) {
        char*  pz;
        /*
         *  'pzScan' is pointing to the first character on a line.
         *  Check for a directive on the current line before scanning
         *  later lines.
         */
        if (*pzScan == '#')
            pz = ++pzScan;
        else {
            pz = strstr( pzScan, zCheckList );
            if (pz == NULL)
                AG_ABEND( aprf( zNoEndif, pCurCtx->pzFileName,
                                pCurCtx->lineNo ));

            pzScan = pz + STRSIZE( zCheckList );
        }

        while (isspace( *pzScan )) pzScan++;

        switch (findDirective( pzScan )) {
        case DIR_ENDIF:
        {
            /*
             *  We found the endif we are interested in
             */
            char* pz = strchr( pzScan, '\n' );
            if (pz != NULL)
                return pz+1;
            return pzScan + strlen( pzScan );
        }

        case DIR_IFDEF:
        case DIR_IFNDEF:
            /*
             *  We found a nested ifdef/ifndef
             */
            pzScan = skipToEndif( pzScan );
            break;

        default:
            /*
             *  We do not care what we found
             */
            break; /* ignore it */
        }  /* switch (findDirective( pzScan )) */
    }
}


/*
 *  skipToElseEnd
 *
 *  Skip through the text to a matching "#endif" or "#else" or
 *  "#elif*def".  We do this when we are skipping code due to a failed
 *  "#if*def" test.
 */
STATIC char*
skipToElseEnd( char* pzScan )
{
    for (;;) {
        char*  pz;
        /*
         *  'pzScan' is pointing to the first character on a line.
         *  Check for a directive on the current line before scanning
         *  later lines.
         */
        if (*pzScan == '#')
            pz = ++pzScan;
        else {
            pz = strstr( pzScan, zCheckList );
            if (pz == NULL)
                AG_ABEND( aprf( zNoEndif, pCurCtx->pzFileName,
                                pCurCtx->lineNo ));

            pzScan = pz + STRSIZE( zCheckList );
        }

        while (isspace( *pzScan )) pzScan++;

        switch (findDirective( pzScan )) {
        case DIR_ELSE:
            /*
             *  We found an "else" directive for an "ifdef"/"ifndef"
             *  that we were skipping over.  Start processing the text.
             */
            ifdefLevel++;
            /* FALLTHROUGH */

        case DIR_ENDIF:
        {
            /*
             *  We reached the end of the "ifdef"/"ifndef" we were
             *  skipping (or we dropped in from above).
             *  Start processing the text.
             */
            char* pz = strchr( pzScan, '\n' );
            if (pz != NULL)
                return pz+1;
            return pzScan + strlen( pzScan );
        }

        case DIR_IFDEF:
        case DIR_IFNDEF:
            /*
             *  We have found a nested "ifdef"/"ifndef".
             *  Call "skipToEndif()" to find *its* end, then
             *  resume looking for our own "endif" or "else".
             */
            pzScan = skipToEndif( pzScan );
            break;

        default:
            /*
             *  We either don't know what it is or we do not care.
             */
            break;
        }  /* switch (findDirective( pzScan )) */
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Special routines for each directive.  These routines are *ONLY*
 *  called from the table when the input is being processed.
 *  After this routine are either declarations or definitions of
 *  directive handling routines.  The documentation for these routines
 *  is extracted from this file.  See 'makedef.sh' for how it works.
 *  Each declared directive should have either a 'dummy:' section
 *  (if the directive is to be ignored) or a 'text:' section
 *  (if there is some form of implementation).  If the directive
 *  needs or may take arguments (e.g. '#define'), then there should
 *  also be an 'arg:' section describing the argument(s).
 */
STATIC char*
doDir_IGNORE( char* pzArg, char* pzScan )
{
    return pzScan;
}


/*=directive assert
 *
 *  dummy:  Assert directives are ignored.
=*/


/*=directive define
 *
 *  arg:  name [ <text> ]
 *
 *  text:
 *  Will add the name to the define list as if it were a DEFINE program
 *  argument.  Its value will be the first non-whitespace token following
 *  the name.  Quotes are @strong{not} processed.
 *
 *  After the definitions file has been processed, any remaining entries
 *  in the define list will be added to the environment.
=*/
STATIC char*
doDir_define( char* pzArg, char* pzScan )
{
    char*  pzName = pzArg;

    /*
     *  Skip any #defines that do not look reasonable
     */
    if (! isalpha( *pzArg ))
        return pzScan;
    while (ISNAMECHAR( *pzArg )) pzArg++;

    /*
     *  IF this is a macro definition (rather than a value def),
     *  THEN we will ignore it.
     */
    if (*pzArg == '(')
        return pzScan;

    /*
     *  We have found the end of the name.
     *  IF there is no more data on the line,
     *  THEN we do not have space for the '=' required by PUTENV.
     *       Therefore, move the name back over the "#define"
     *       directive itself, giving us the space needed.
     */
    if (! isspace( *pzArg )) {
        pzName--;
        *pzArg = NUL;
        strcpy( pzName, pzName+1 );
        strcat( pzName, "=" );

    } else {
        /*
         *  Otherwise, insert the '=' and move any data up against it.
         *  We only accept one space separated token.  We are not
         *  ANSI-C compliant.  ;-)
         */
        char*  pz = pzArg+1;
        *pzArg++ = '=';
        while (isspace( *pz )) pz++;

        for (;;) {
            if ((*pzArg++ = *pz++) == NUL)
                break;
            if (isspace( *pz )) {
                *pzArg = NUL;
                break;
            }
        }
    }

    SET_OPT_DEFINE( pzName );
    return pzScan;
}


/*=directive elif
 *
 *  text:
 *  This must follow an @code{#if}
 *  otherwise it will generate an error.
 *  It will be ignored.
=*/
STATIC char*
doDir_elif( char* pzArg, char* pzScan )
{
    tSCC z[] =
        "`#elif' directive encountered out of context\n\tin %s on line %d\n";

    AG_ABEND( aprf( z, pCurCtx->pzFileName, pCurCtx->lineNo ));
}


/*=directive else
 *
 *  text:
 *  This must follow an @code{#if}, @code{#ifdef} or @code{#ifndef}.
 *  If it follows the @code{#if}, then it will be ignored.  Otherwise,
 *  it will change the processing state to the reverse of what it was.
=*/
STATIC char*
doDir_else( char* pzArg, char* pzScan )
{
    if (--ifdefLevel < 0)
        AG_ABEND( aprf( zNoMatch, pCurCtx->pzFileName, pCurCtx->lineNo,
                        "else" ));

    return skipToEndif( pzScan );
}


/*=directive endif
 *
 *  text:
 *  This must follow an @code{#if}, @code{#ifdef} or @code{#ifndef}.
 *  In all cases, this will resume normal processing of text.
=*/
STATIC char*
doDir_endif( char* pzArg, char* pzScan )
{
    if (--ifdefLevel < 0)
        AG_ABEND( aprf( zNoMatch, pCurCtx->pzFileName, pCurCtx->lineNo,
                        "endif" ));

    return pzScan;
}


/*=directive endshell
 *
 *  text:
 *  Ends the text processed by a command shell into autogen definitions.
=*/
STATIC char*
doDir_endshell( char* pzArg, char* pzScan )
{
    /*
     *  In actual practice, the '#endshell's must be consumed inside
     *  the 'doDir_shell()' procedure.
     */
    AG_ABEND( aprf( zNoMatch, pCurCtx->pzFileName, pCurCtx->lineNo,
                    "endshell" ));
}


/*=directive error
 *
 *  arg:  [ <descriptive text> ]
 *
 *  text:
 *  This directive will cause AutoGen to stop processing
 *  and exit with a status of EXIT_FAILURE.
=*/
STATIC char*
doDir_error( char* pzArg, char* pzScan )
{
    AG_ABEND( aprf( "#error directive -- in %s on line %d\n\t%s\n",
                    pCurCtx->pzFileName, pCurCtx->lineNo, pzArg ));
}


/*=directive ident
 *
 *  dummy:  Ident directives are ignored.
=*/


/*=directive if
 *
 *  arg:  [ <ignored conditional expression> ]
 *
 *  text:
 *  @code{#if} expressions are not analyzed.  @strong{Everything} from here
 *  to the matching @code{#endif} is skipped.
=*/
STATIC char*
doDir_if( char* pzArg, char* pzScan )
{
    return skipToEndif( pzScan );
}


/*=directive ifdef
 *
 *  arg:  name-to-test
 *
 *  text:
 *  The definitions that follow, up to the matching @code{#endif} will be
 *  processed only if there is a corresponding @code{-Dname} command line
 *  option.
=*/
STATIC char*
doDir_ifdef( char* pzArg, char* pzScan )
{
    if (getDefine( pzArg ) == NULL)
        return skipToElseEnd( pzScan );
    ifdefLevel++;
    return pzScan;
}


/*=directive ifndef
 *
 *  arg:  name-to-test
 *
 *  text:
 *  The definitions that follow, up to the matching @code{#endif} will be
 *  processed only if there is @strong{not} a corresponding @code{-Dname}
 *  command line option or there was a canceling @code{-Uname} option.
=*/
STATIC char*
doDir_ifndef( char* pzArg, char* pzScan )
{
    if (getDefine( pzArg ) != NULL)
        return skipToElseEnd( pzScan );
    ifdefLevel++;
    return pzScan;
}


/*=directive include
 *
 *  arg:  unadorned-file-name
 *
 *  text:
 *  This directive will insert definitions from another file into
 *  the current collection.  If the file name is adorned with
 *  double quotes or angle brackets (as in a C program), then the
 *  include is ignored.
=*/
STATIC char*
doDir_include( char* pzArg, char* pzScan )
{
    tSCC*      apzSfx[] = { "def", NULL };
    tScanCtx*  pCtx;
    size_t     inclSize;
    char       zFullName[ MAXPATHLEN + 1 ];

    /*
     *  Ignore C-style includes.  This allows "C" files to be processed
     *  for their "#define"s.
     */
    if ((*pzArg == '"') || (*pzArg == '<'))
        return pzScan;
    pCurCtx->pzScan  = pzScan;

    if (! SUCCESSFUL( findFile( pzArg, zFullName, apzSfx ))) {
        tSCC zFmt[] = "WARNING:  cannot find `%s' definitions file\n";
        fprintf( pfTrace, zFmt, pzArg );
        return pzScan;
    }

    /*
     *  Make sure the specified file is a regular file and we can get
     *  the correct size for it.
     */
    {
        struct stat stbf;
        if (stat( zFullName, &stbf ) != 0) {
            fprintf( pfTrace, "WARNING %d (%s):  cannot stat `%s' "
                     "for include\n", errno, strerror( errno ), zFullName );
            return pzScan;
        }
        if (! S_ISREG( stbf.st_mode )) {
            fprintf( pfTrace, "WARNING:  `%s' must be regular file to "
                     "include\n", zFullName );
            return pzScan;
        }
        inclSize = stbf.st_size;
        if (outTime < stbf.st_mtime)
            outTime = stbf.st_mtime + 1;
    }
    if (inclSize == 0)
        return pzScan;

    /*
     *  Get the space for the output data and for context overhead.
     *  This is an extra allocation and copy, but easier than rewriting
     *  'loadData()' for this special context.
     */
    {
        size_t sz = sizeof( tScanCtx ) + 4 + inclSize;
        pCtx = (tScanCtx*)AGALOC( sz, "include def header" );

        memset( (void*)pCtx, 0, sz );
        pCtx->lineNo = 1;
    }

    /*
     *  Link it into the context stack
     */
    pCtx->pCtx       = pCurCtx;
    pCurCtx          = pCtx;
    AGDUPSTR( pCtx->pzFileName, zFullName, "def file name" );

    pCtx->pzScan     =
    pCtx->pzData     =
    pzScan           = (char*)(pCtx + 1);

    /*
     *  Read all the data.  Usually in a single read, but loop
     *  in case multiple passes are required.
     */
    {
        FILE*  fp = fopen( zFullName, "r" FOPEN_TEXT_FLAG );
        char*  pz = pzScan;

        if (fp == NULL)
            AG_ABEND( aprf( zCannot, errno, "open file", zFullName,
                            strerror( errno )));

        do  {
            size_t rdct = fread( (void*)pz, 1, inclSize, fp );

            if (rdct == 0)
                AG_ABEND( aprf( zCannot, errno, "read file", zFullName,
                                strerror( errno )));

            pz += rdct;
            inclSize -= rdct;
        } while (inclSize > 0);

        fclose( fp );
        *pz = NUL;
    }

    return pzScan;
}


/*=directive line
 *
 *  text:
 *
 *  Alters the current line number and/or file name.  You may wish to
 *  use this directive if you extract definition source from other files.
 *  @command{getdefs} uses this mechanism so AutoGen will report the correct
 *  file and approximate line number of any errors found in extracted
 *  definitions.
=*/
STATIC char*
doDir_line( char* pzArg, char* pzScan )
{
    /*
     *  The sequence must be:  #line <number> "file-name-string"
     *
     *  Start by scanning up to and extracting the line number.
     */
    while (isspace( *pzArg )) pzArg++;
    if (! isdigit( *pzArg ))
        return pzScan;

    pCurCtx->lineNo = strtol( pzArg, &pzArg, 0 );

    /*
     *  Now extract the quoted file name string.
     *  We dup the string so it won't disappear on us.
     */
    while (isspace( *pzArg )) pzArg++;
    if (*(pzArg++) != '"')
        return pzScan;
    {
        char* pz = strchr( pzArg, '"' );
        if (pz == NULL)
            return pzScan;
        *pz = NUL;
    }
    AGFREE( (void*)pCurCtx->pzFileName );
    AGDUPSTR( pCurCtx->pzFileName, pzArg, "#line file name" );

    return pzScan;
}


/*=directive option
 *
 *  arg:  opt-name [ <text> ]
 *
 *  text:
 *
 *  This directive will pass the option name and associated text to the
 *  AutoOpts optionLoadLine routine (@pxref{optionLoadLine}).  The option text
 *  may span multiple lines by continuing them with a backslash.
 *  The backslash/newline pair will be replaced with two space characters.
 *  This directive may be used to set a search path for locating template files
 *  For example, this:
 *
 *  @example
 *    #option templ-dirs $ENVVAR/dirname
 *  @end example
 *  @noindent
 *  will direct autogen to use the @code{ENVVAR} environment variable to find
 *  a directory named @code{dirname} that (may) contain templates.  Since these
 *  directories are searched in most recently supplied first order, search
 *  directories supplied in this way will be searched before any supplied on
 *  the command line.
=*/
STATIC char*
doDir_option( char* pzArg, char* pzScan )
{
    optionLoadLine( &autogenOptions, pzArg );
    return pzScan;
}


/*=directive pragma
 *
 *  dummy:  pragma directives are ignored.
=*/


/*=directive shell
 *
 *  text:
 *  Invokes @code{$SHELL} or @file{/bin/sh} on a script that should
 *  generate AutoGen definitions.  It does this using the same server
 *  process that handles the back-quoted @code{`} text.
 *  @strong{CAUTION}@:  let not your @code{$SHELL} be @code{csh}.
=*/
STATIC char*
doDir_shell( char* pzArg, char* pzScan )
{
    tSCC       zShellText[] = "Computed Definitions";
    tSCC       zEndShell[]  = "\n#endshell";

    tScanCtx*  pCtx;
    char*      pzText = pzScan;

    /*
     *  The output time will always be the current time.
     *  The dynamic content is always current :)
     */
    outTime = time( NULL );

    /*
     *  IF there are no data after the '#shell' directive,
     *  THEN we won't write any data
     *  ELSE we have to find the end of the data.
     */
    if (strncmp( pzText, zEndShell+1, STRSIZE( zEndShell )-1) == 0)
        return pzScan;

    pzScan = strstr( pzScan, zEndShell );
    if (pzScan == NULL)
        AG_ABEND( aprf( "Missing #endshell after '#shell' in %s on line %d\n",
                        pCurCtx->pzFileName, pCurCtx->lineNo ));

    *pzScan = NUL;

    /*
     *  Advance the scan pointer to the next line after '#endshell'
     *  IF there is no such line,
     *  THEN the scan will resume on a zero-length string.
     */
    pzScan = strchr( pzScan + STRSIZE( zEndShell ), '\n' );
    if (pzScan == NULL)
        pzScan = "";

    /*
     *  Save the scan pointer into the current context
     */
    pCurCtx->pzScan  = pzScan;

    if (pzShellProgram == NULL)
        pzShellProgram = getDefine( zShellEnv );

    /*
     *  Run the shell command.  The output text becomes the
     *  "file text" that is used for more definitions.
     */
    pzText = runShell( pzText );
    if (  (pzText == NULL)
       || (*pzText == NUL))
        return pzScan;

    /*
     *  Get the space for the output data and for context overhead.
     *  This is an extra allocation and copy, but easier than rewriting
     *  'loadData()' for this special context.
     */
    pCtx = (tScanCtx*)AGALOC( sizeof( tScanCtx ) + strlen( pzText ) + 4,
                              "shell output" );

    /*
     *  Link the new scan data into the context stack
     */
    pCtx->pCtx       = pCurCtx;
    pCurCtx          = pCtx;

    /*
     *  Set up the rest of the context structure
     */
    AGDUPSTR( pCtx->pzFileName, zShellText, "shell text" );
    pCtx->pzScan     =
    pCtx->pzData     = (char*)(pCtx+1);
    pCtx->lineNo     = 0;
    strcpy( pCtx->pzScan, pzText );
    AGFREE( pzText );

    return pCtx->pzScan;
}


/*=directive undef
 *
 *  arg:  name-to-undefine
 *
 *  text:
 *  Will remove any entries from the define list
 *  that match the undef name pattern.
=*/
STATIC char*
doDir_undef( char* pzArg, char* pzScan )
{
    SET_OPT_UNDEFINE( pzArg );
    return pzScan;
}

/*+++ End of Directives +++*/
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defDirect.c */
