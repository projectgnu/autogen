
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opts.h"

struct print_list {
    char**     papz;
    int        index;
};

typedef struct print_list tPrintList, *tpPrintList;

typedef int (tCompProc)(const void*, const void*);
tCompProc compProc;

int maxEntryWidth = 0;
int fillColumnCt  = 0;

char zLine[ 133 ];
char zFmtLine[ 133 ];

char**  papzLines = (char**)NULL;
size_t  allocCt   = 0;
size_t  usedCt    = 0;
size_t  lineWidth = 79;
size_t  columnCt  = 0;
size_t  columnSz  = 0;

void readLines( void );
void writeRows( void );
void writeColumns( void );


    int
main( int    argc,
      char** argv )
{
    if (optionProcess( &columnsOptions, argc, argv ) != argc) {
        fputs( "Error:  this program takes no arguments\n", stderr );
        USAGE( EXIT_FAILURE );
    }

    if (HAVE_OPT( LINE_WIDTH ))
        lineWidth = OPT_VALUE_LINE_WIDTH;

    if (HAVE_OPT( COL_WIDTH ))
        columnSz = OPT_VALUE_COL_WIDTH;

    if (HAVE_OPT( COLUMNS ))
        columnCt = OPT_VALUE_COLUMNS;

    readLines();

    if (HAVE_OPT( SORT ))
        qsort( (void*)papzLines, usedCt, sizeof( char* ), &compProc );

    if (HAVE_OPT( BY_COLUMNS ))
         writeColumns();
    else writeRows();

    return 0;
}


    void
readLines( void )
{
    int   sepLen;
    char* pzText;

    if (HAVE_OPT( SEPARATION ))
         sepLen = strlen( OPT_ARG( SEPARATION ));
    else sepLen = 0;


    /*
     *  Read the input text, stripping trailing white space
     */
    for (;;) {
        char*   pzL;
        char*   pzText = fgets( zLine, sizeof( zLine ), stdin );
        int     len;

        if (pzText == (char*)NULL)
            break;

        /*
         *  Trim off trailing white space.
         */
        len = strlen( pzText );
        pzText += len;
        while ((pzText > zLine) && isspace( pzText[-1] )) {
            pzText--;
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
            len = sprintf( zFmtLine, OPT_ARG( FORMAT ), zLine );
        } else {
            pzText = zLine;
        }

        /*
         *  Allocate a string and space in the pointer array.
         */
        pzL = (char*)malloc( len+1+sepLen );
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
    }

    if (maxEntryWidth++ == 0) {
        fputs( "Warning:  no input text was read\n", stderr );
        exit( EXIT_SUCCESS );
    }

    /*
     *  Set the line width to the amount of space we have to play with.
     *  OPT_VALUE_INDENT defaults to zero.
     */
    lineWidth -= OPT_VALUE_INDENT;
    if (lineWidth < maxEntryWidth)
        lineWidth = maxEntryWidth;

    /*
     *  If we do not have a column size set,
     *  then figure out what it must be.
     */
    if (columnSz == 0) {
        /*
         *  IF the column count is set, then that dictates it.
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
            else
                columnCt = (spreadwidth / maxEntryWidth) + 1;
        }

        /*
         *  ELSE neither the column size or count have been set:
         */
        else {
            columnCt = ((lineWidth - maxEntryWidth) / maxEntryWidth) + 1;
            columnSz = ((lineWidth - maxEntryWidth) / columnCt);
        }
    }

    /*
     *  Otherwise, the column size has been set.  Ensure it is sane.
     */
    else {
        if (maxEntryWidth > columnSz)
            columnSz = maxEntryWidth;

        if (  (columnCt == 0)
           || ( ((columnSz * (columnCt-1)) + maxEntryWidth) > lineWidth ))
            columnCt = ((lineWidth - maxEntryWidth) / columnSz) + 1;
    }

    /*
     *  Make sure the output line length is big enough
     */
    if (lineWidth < maxEntryWidth) {
        fprintf( stderr, "Warning:  line width of %3d is smaller than\n"
                         "          longest line  %3d\n",
                 lineWidth, maxEntryWidth );
        lineWidth = maxEntryWidth;
    }
}


    void
writeColumns( void )
{
    char zFmt[ 12 ];
    int  colCt, rowCt, col, row;
    tpPrintList pPL;

    colCt = columnCt;
    sprintf( zFmt, "%%-%ds", columnSz );

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
        sprintf( zFmt, "%%-%ds", fsz );
    }


    for ( row = 0 ;; ) {
        char*  pzL;
        char*  pzE;

        if (OPT_VALUE_INDENT > 0) {
            int ct = OPT_VALUE_INDENT;
            do { fputc( ' ', stdout ); } while (--ct > 0);
        }

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
         */
        while (++col < colCt) {
            pzL = *(pPL[col-1].papz++);
            fprintf( stdout, zFmt, pzL );
            free( (void*)pzL );
        }

        /*
         *  Print the last entry on the line, without the width format.
         *  IF it is also the last entry, see if we should strip the
         *  separation string.
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

        fputs( pzE, stdout );
        fputc( '\n', stdout );
        free( (void*)pzE );
    }
}


    void
writeRows( void )
{
    char zFmt[ 12 ];
    int  colCt;

    colCt = columnCt;
    sprintf( zFmt, "%%-%ds", columnSz );

    /*
     *  IF we have a separator,
     *  THEN remove it from the last entry.
     */
    if (HAVE_OPT( SEPARATION )) {
        char* pz = papzLines[ usedCt-1 ];
        pz += strlen( pz ) - strlen( OPT_ARG( SEPARATION ));
        *pz = '\0';
    }

    if (OPT_VALUE_INDENT > 0) {
        int ct = OPT_VALUE_INDENT;
        do { fputc( ' ', stdout ); } while (--ct > 0);
    }

    {
        char**  ppzLL = papzLines;
        size_t  left  = usedCt;
        int     lnNo  = 0;

        for (;;) {
            char* pzL = *ppzLL++;
            if (--left <= 0) {
                fputs( pzL, stdout );
                fputc( '\n', stdout );
                free( (void*)pzL );
                break;
            }

            if (++lnNo < colCt)
                fprintf( stdout, zFmt, pzL );

            else {
                lnNo = 0;
                fputs( pzL, stdout );
                fputc( '\n', stdout );
                if (OPT_VALUE_INDENT > 0) {
                    int ct = OPT_VALUE_INDENT;
                    do { fputc( ' ', stdout ); } while (--ct > 0);
                }
            }

            free( (void*)pzL );
        } while (left > 0);
    }
}


    int
compProc( const void* p1, const void* p2 )
{
    char* pz1 = *(char**)p1;
    char* pz2 = *(char**)p2;
    return strcmp( pz1, pz2 );
}
