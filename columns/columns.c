
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opts.h"

typedef int (tCompProc)(const void*, const void*);
tCompProc compProc;

int maxline = 0;
char zLine[ 133 ];
char zFmtLine[ 133 ];

char**  papzLines = (char**)NULL;
size_t  allocCt   = 0;
size_t  usedCt    = 0;


void readLines( void );
void writeLines( void );


    int
main( int    argc,
      char** argv )
{
    if (optionProcess( &columnsOptions, argc, argv ) != argc) {
        fputs( "Error:  this program takes no arguments\n", stderr );
        USAGE( EXIT_FAILURE );
    }

    readLines();

    if (HAVE_OPT( SORT ))
        qsort( (void*)papzLines, usedCt, sizeof( char* ), &compProc );

    writeLines();

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

        if (len > maxline)
            maxline = len;
    }

    if (maxline++ == 0) {
        fputs( "Warning:  no input text was read\n", stderr );
        exit( EXIT_SUCCESS );
    }

    /*
     *  Set the line width to the amount of space we have to play with.
     *  OPT_VALUE_INDENT defaults to zero.
     */
    OPT_VALUE_LINE_WIDTH -= OPT_VALUE_INDENT;

    /*
     *  Raise the line width if it is supposed to be longer than
     *  the longest line we found in our input
     */
    if (OPT_VALUE_COL_WIDTH > maxline)
        maxline = OPT_VALUE_COL_WIDTH;

    /*
     *  On the other hand, if the number of columns was specified,
     *  then maximize the separation of the columns.
     */
    else if (OPT_VALUE_COLUMNS > 0) {
        /*
         *  IF there is to be only one column,
         *  THEN its nominal "width" is the full line width.
         *       (Why is this person bothering with this program?)
         */
        if (OPT_VALUE_COLUMNS == 1)
            maxline = OPT_VALUE_LINE_WIDTH;

        /*
         *  ELSE we will put any extra space between the columns
         *       Compute the per-column space for all but the
         *       last column (we will not insert following white space)
         */
        else {
            int  spreadwidth = OPT_VALUE_LINE_WIDTH - maxline;
            int  fsz = spreadwidth / (OPT_VALUE_COLUMNS-1);

            /*
             *  IF there is room for added space,
             *  THEN add it.
             */
            if (fsz > maxline)
                maxline = fsz;
        }
    }

    /*
     *  Make sure the output line length is big enough
     */
    if (OPT_VALUE_LINE_WIDTH < maxline) {
        fprintf( stderr, "Warning:  line width of %3d is smaller than\n"
                         "          longest line  %3d\n",
                 OPT_VALUE_LINE_WIDTH, maxline );
        OPT_VALUE_LINE_WIDTH = maxline;
    }
}


    void
writeLines( void )
{
    char zFmt[ 12 ];
    int  colCt;

    /*
     *  Compute the columizing formatting string.
     *  If the column width has been specified,
     *  then 'maxline' is set to the right size already.
     *  Otherwise, we will try to insert any left over space
     *  between columns.
     */
    {
        int  rem;
        int  fsz;

        colCt = OPT_VALUE_LINE_WIDTH / maxline;
        rem   = OPT_VALUE_LINE_WIDTH - (colCt * maxline);

        if ((rem == 0) || (colCt < 2) || (OPT_VALUE_COL_WIDTH > 0))
            fsz = maxline;
        else
            fsz = maxline + (rem / (colCt-1));
        sprintf( zFmt, "%%-%ds", fsz );
    }

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

        do  {
            char* pzL = *ppzLL++;
            left--;

            if (++lnNo < colCt)
                fprintf( stdout, zFmt, pzL );

            else {
                lnNo = 0;
                fputs( pzL, stdout );
                fputc( '\n', stdout );
                if ((OPT_VALUE_INDENT > 0) && (left > 0)) {
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
