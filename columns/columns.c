
/*
 *  columns.c
 *  $Id: columns.c,v 4.9 2006/09/23 00:51:11 bkorb Exp $
 */

/*
 *  Columns copyright 1992-2006 Bruce Korb
 *
 *  Columns is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Columns is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Columns.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */
#ifndef NUL
# define NUL '\0'
#endif

struct print_list {
    char**     papz;
    int        index;
};

typedef struct print_list tPrintList, *tpPrintList;

int maxEntryWidth = 0;
int fillColumnCt  = 0;

char zLine[ 133 ];
char zFmtLine[ 133 ];

char**  papzLines  = (char**)NULL;
tCC*    pzLinePfx  = NULL;
tCC*    pzFirstPfx = NULL;
size_t  allocCt    = 0;
size_t  usedCt     = 0;
size_t  lineWidth  = 79;
size_t  columnCt   = 0;
size_t  columnSz   = 0;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static uint32_t
handleIndent( tCC* pzIndentArg );

static void
readLines( void );

static void
writeColumns( void );

static void
writeRows( void );

static int
compProc( const void* p1, const void* p2 );
/* = = = END-STATIC-FORWARD = = = */

int
main( int    argc,
      char** argv )
{
    if (optionProcess( &columnsOptions, argc, argv ) != argc) {
        fputs( "Error:  this program takes no arguments\n", stderr );
        USAGE( EXIT_FAILURE );
    }

    if (HAVE_OPT( WIDTH ))
        lineWidth = OPT_VALUE_WIDTH;

    if (HAVE_OPT( INDENT )) {
        size_t indentSize = handleIndent( OPT_ARG( INDENT ));
        lineWidth -= indentSize;

        if (! HAVE_OPT( FIRST_INDENT ))
            pzFirstPfx = pzLinePfx;
        else {

            /*
             *  The first line has a special indentation/prefix.
             *  Compute it, but do not let it be larger than
             *  the indentation value.
             */
            tCC* pzSave = pzLinePfx;
            size_t firstSize = handleIndent( OPT_ARG( FIRST_INDENT ));
            pzFirstPfx = pzLinePfx;
            pzLinePfx  = pzSave;

            /*
             *  Now force the first line prefix to have the same size
             *  as the indentSize
             */
            if (firstSize > indentSize) {
                char* p = malloc( indentSize + 1 );
                strncpy( p, pzFirstPfx, indentSize );
                p[ indentSize ] = NUL;
                pzFirstPfx = p;
                fprintf( stderr, "Warning: prefix `%s' has been truncated to ",
                         pzFirstPfx );
                fprintf( stderr, "`%s'\n", pzFirstPfx );

            } else if (firstSize < indentSize) {
                char* tmp = malloc( indentSize + 1 );
                char  z[10];
                snprintf( z, sizeof(z), "%%-%ds", indentSize );
                snprintf( tmp, indentSize + 1, z, pzFirstPfx );
                pzFirstPfx = tmp;
            }
        }
    }

    if (HAVE_OPT( LINE_SEPARATION ))
        lineWidth -= strlen( OPT_ARG( LINE_SEPARATION ));

    if (HAVE_OPT( COL_WIDTH ))
        columnSz = OPT_VALUE_COL_WIDTH;

    if (HAVE_OPT( COLUMNS ))
        columnCt = OPT_VALUE_COLUMNS;

    if (lineWidth <= 16)
        lineWidth = 16;

    readLines();

    if (HAVE_OPT( SORT ))
        qsort( (void*)papzLines, usedCt, sizeof( char* ), &compProc );

    if (HAVE_OPT( BY_COLUMNS ))
         writeColumns();
    else writeRows();

    return EXIT_SUCCESS;
}


static uint32_t
handleIndent( tCC* pzIndentArg )
{
    char* pz;
    uint32_t colCt = strtoul( pzIndentArg, &pz, 0 );

    /*
     *  IF the indent argument is a number
     */
    if (*pz == '\0') {
        char* p;

        /*
         *  AND that number is reasonable, ...
         */
        if (colCt <= 0)
            return 0;

        /*
         *  Allocate a string to hold the line prefix
         */
        pzLinePfx = p = malloc( colCt + 1 );
        if (pzLinePfx == NULL) {
            fprintf( stderr, "Cannot malloc %d bytes\n", colCt + 1 );
            exit( EXIT_FAILURE );
        }

        /*
         *  Set it to a NUL terminated string of spaces
         */
        memset( p, ' ', colCt );
        p[ colCt ] = '\0';

    } else {
        tCC* p;
        /*
         *  Otherwise, set the line prefix to whatever the string is.
         *  It will not be the empty string because that is handled
         *  as an indent count of zero and is ignored.
         */
        pzLinePfx = pzIndentArg;
        p = pzIndentArg;
        colCt =  0;
        for (;;) {
            /*
             *  Figure out how much of the line is taken up by
             *  the prefix.  We might consider restricted format
             *  strings some time in the future, but not now.
             */
            switch (*p++) {
            case '\0':
                goto colsCounted;

            case '\t':
                colCt += OPT_VALUE_TAB_WIDTH;
                colCt -= (colCt % OPT_VALUE_TAB_WIDTH);
                break;

            case '\n':
            case '\f':
            case '\r':
                colCt = 0;
                break;

            default:
                colCt++;
                break;

            case '\a':
                break;
            }
        } colsCounted:;
    }

    return colCt;
}


static void
readLines( void )
{
    int   sepLen;

    if (HAVE_OPT( SEPARATION ))
         sepLen = strlen( OPT_ARG( SEPARATION ));
    else sepLen = 0;


    /*
     *  Read the input text, stripping trailing white space
     */
    for (;;) {
        char*     pzL;
        char*     pzText = fgets( zLine, sizeof( zLine ), stdin );
        uint32_t  len;

        if (pzText == NULL)
            break;

        /*
         *  Trim off trailing white space.
         */
        len = strlen( pzText );
        pzText += len;
        while (isspace( pzText[-1] )) {
            if (--pzText == zLine)
                goto next_line;
            len--;
        }

        *pzText = '\0';

        /*
         *  IF the input lines are to be reformatted,
         *  THEN the length is the result of the sprintf
         *  Else, compute the length.
         */
        if (HAVE_OPT( FORMAT )) {
            pzText = zFmtLine;
            len = snprintf( zFmtLine, sizeof(zFmtLine), OPT_ARG( FORMAT ),
                            zLine );
        } else {
            pzText = zLine;
        }

        /*
         *  Allocate a string and space in the pointer array.
         */
        len += sepLen + 1;
        pzL = (char*)malloc( len );
        if (++usedCt > allocCt) {
            allocCt += 128;
            papzLines = (char**)realloc( (void*)papzLines,
                                         sizeof( char* ) * allocCt );
        }
        papzLines[ usedCt-1 ] = pzL;

        /*
         *  Copy the text and append the separation character
         */
        strcpy( pzL, pzText );

        /*
         *  Initially, all strings have the separator,
         *  the entries may get reordered.
         */
        if (sepLen > 0)
            strcat( pzL, OPT_ARG( SEPARATION ));

        if (len > maxEntryWidth)
            maxEntryWidth = len;
    next_line:;
    }

    if (maxEntryWidth == 0) {
        fputs( "Warning:  no input text was read\n", stderr );
        exit( EXIT_SUCCESS );
    }

    /*
     *  Set the line width to the amount of space we have to play with.
     */
    if (lineWidth < maxEntryWidth)
        lineWidth = maxEntryWidth;

    /*
     *  If we do not have a column size set,
     *  then figure out what it must be.
     */
    if (columnSz == 0) {
        /*
         *  IF the column count has not been set,
         *  THEN compute it.
         */
        if (columnCt == 0)
            columnCt = lineWidth / maxEntryWidth;

        /*
         *  IF there are to be multiple columns, ...
         */
        if (columnCt > 1) {
            int  spreadwidth = lineWidth - maxEntryWidth;
            int  sz = spreadwidth / (columnCt-1);

            /*
             *  Either there is room for added space,
             *  or we must (possibly) reduce the number of columns
             */
            if (sz >= maxEntryWidth)
                columnSz = sz;
            else {
                columnCt = (spreadwidth / maxEntryWidth) + 1;
                if (columnCt > 1)
                     columnSz = spreadwidth / (columnCt - 1);
                else columnSz = lineWidth;
            }
        }
    }

    /*
     *  Otherwise, the column size has been set.  Ensure it is sane.
     */
    else {
        /*
         *  Increase the column size to the width of the widest entry
         */
        if (maxEntryWidth > columnSz)
            columnSz = maxEntryWidth;

        /*
         *  IF     we have not been provided a column count
         *    *OR* we are set to overfill the output line,
         *  THEN compute the number of columns.
         */
        if (  (columnCt == 0)
           || ( ((columnSz * (columnCt-1)) + maxEntryWidth) > lineWidth ))
            columnCt = ((lineWidth - maxEntryWidth) / columnSz) + 1;
    }

    /*
     *  Ensure that any "spread" we added to the column size
     *  does not exceed the parameterized limit.
     */
    if (   HAVE_OPT( SPREAD )
        && ((maxEntryWidth + OPT_VALUE_SPREAD - 1) < columnSz))
        columnSz = maxEntryWidth + OPT_VALUE_SPREAD - 1;
}


static void
writeColumns( void )
{
    char zFmt[ 12 ];
    int  colCt, rowCt, col, row;
    tpPrintList pPL;

    colCt = columnCt;
    snprintf( zFmt, sizeof(zFmt), "%%-%ds", columnSz );

    if (colCt == 1) {
        writeRows();
        return;
    }

    pPL   = (tpPrintList)malloc( colCt * sizeof( tPrintList ));

    /*
     *  This "loop" is normally executed half way through and exited.
     *  IF, however, we would  produce an empty final column,
     *  we will reduce our column count and line width and then
     *  try the top-of-column pointer computation again.
     *
     *  The problem solved here is that sometimes, when the
     *  number of entries in a row is greater than the number of rows,
     *  it is possible that all the entries that would have been
     *  in the last column are, instead, essentially put on the
     *  last row.  That will leave the final column empty.
     *  We could regroup at that point and spread the columns some more,
     *  but, if done, is an exercise for later.
     */
    for (;;) {
        int  rem;
        int  fsz;

        rowCt = (usedCt/colCt) + ((usedCt % colCt) ? 1 : 0);

        /*
         *  FOR each column, compute the address of the pointer to
         *  the string at the top of the column, and the index of
         *  that entry.
         */
        for (col = 0; col < colCt ; col++) {
            pPL[col].papz  = papzLines + (col * rowCt);
            pPL[col].index = col * rowCt;
        }

        /*
         *  IF the final column is not empty,
         *  THEN break out and start printing.
         */
        if (pPL[colCt-1].index < usedCt)
            break;

        /*
         *  The last column is blank, so we reduce our column count,
         *  even if the user specified a count!!
         */
        colCt--;
        rem   = lineWidth - (colCt * maxEntryWidth);

        if ((rem == 0) || (colCt < 2) || (columnSz > 0))
            fsz = maxEntryWidth;
        else
            fsz = maxEntryWidth + (rem / (colCt-1));
        snprintf( zFmt, sizeof(zFmt), "%%-%ds", fsz );
    }

    /*
     *  Now, actually print each row...
     */
    for ( row = 0 ;; ) {
        char*  pzL;
        char*  pzE;

        if (pzLinePfx != NULL)
            fputs( pzLinePfx, stdout );

        /*
         *  Increment the index of the current entry in the last column.
         *  IF it goes beyond the end of the entries in use,
         *  THEN reduce our column count.
         */
        if ((pPL[colCt-1].index)++ >= usedCt)
            colCt--;

        /*
         *  Get the address of the string in the last column.
         */
        pzE = *(pPL[colCt-1].papz++);

        col = 0;

        /*
         *  FOR every column except the last,
         *     print the entry with the width format
         *
         *  No need to worry about referring to a non-existent
         *  entry.  Only the last column might have that problem,
         *  and we addressed it above where the column count got
         *  decremented.
         */
        while (++col < colCt) {
            pzL = *(pPL[col-1].papz++);
            fprintf( stdout, zFmt, pzL );
            free( (void*)pzL );
        }

        /*
         *  See if we are on the last row.  If so, then
         *  this is the last entry.  Strip any separation
         *  characters, emit the entry and break out.
         */
        if (++row == rowCt) {
            /*
             *  IF we have a separator,
             *  THEN remove it from the last entry.
             */
            if (HAVE_OPT( SEPARATION )) {
                char* pz = pzE + strlen( pzE )
                         - strlen( OPT_ARG( SEPARATION ));
                *pz = '\0';
            }
            fputs( pzE, stdout );
            fputc( '\n', stdout );
            break;
        }

        /*
         *  Print the last entry on the line, without the width format.
         *  If we have line separation (which does not apply to the last
         *  line), then emit those characters, too.
         */
        fputs( pzE, stdout );
        if (HAVE_OPT( LINE_SEPARATION ))
            fputs( OPT_ARG( LINE_SEPARATION ), stdout );

        fputc( '\n', stdout );
        free( (void*)pzE );
    }
}


static void
writeRows( void )
{
    char zFmt[ 12 ];
    int  colCt;

    colCt = columnCt;
    snprintf( zFmt, sizeof(zFmt), "%%-%ds", columnSz );

    /*
     *  IF we have a separator,
     *  THEN remove it from the last entry.
     */
    if (HAVE_OPT( SEPARATION )) {
        char* pz = papzLines[ usedCt-1 ];
        pz += strlen( pz ) - strlen( OPT_ARG( SEPARATION ));
        *pz = '\0';
    }

    if (pzFirstPfx != NULL) {
        fputs( pzFirstPfx, stdout );
        pzFirstPfx = pzLinePfx;
    }

    {
        char**  ppzLL = papzLines;
        size_t  left  = usedCt;
        int     lnNo  = 0;

        /*
         *  FOR every entry we are to emit, ...
         */
        for (;;) {
            char* pzL = *ppzLL++;

            /*
             *  IF this is the last entry,
             *  THEN emit it and a new line and break out
             */
            if (--left <= 0) {
                fputs( pzL, stdout );
                fputc( '\n', stdout );
                free( (void*)pzL );
                break;
            }

            /*
             *  IF the count of entries on this line is still less
             *     than the number of columns,
             *  THEN emit the padded entry
             *  ELSE ...
             */
            if (++lnNo < colCt)
                fprintf( stdout, zFmt, pzL );

            else {
                lnNo = 0;
                /*
                 *  Last entry on the line.  Emit the string without padding.
                 *  IF we have a line separation string, emit that too.
                 */
                fputs( pzL, stdout );
                if (HAVE_OPT( LINE_SEPARATION ))
                    fputs( OPT_ARG( LINE_SEPARATION ), stdout );

                fputc( '\n', stdout );

                /*
                 *  Start the next line with any required indentation
                 */
                if (pzFirstPfx != NULL) {
                    fputs( pzFirstPfx, stdout );
                    pzFirstPfx = pzLinePfx;
                }
            }

            free( (void*)pzL );
        } while (left > 0);
    }
}


/*
 *  Line comparison procedure
 */
static int
compProc( const void* p1, const void* p2 )
{
    char const* pz1 = *(char* const*)p1;
    char const* pz2 = *(char* const*)p2;
    return strcmp( pz1, pz2 );
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of columns/columns.c */
