/*
 *  $Id: gdemit.c,v 3.5 2003/12/27 15:06:40 bkorb Exp $
 *
 *    getdefs copyright 1999-2003 Bruce Korb
 *
 *  Author:            Bruce Korb <bkorb@gnu.org>
 *  Maintainer:        Bruce Korb <bkorb@gnu.org>
 *  Created:           Sat Dec 1, 2001
 *  Last Modified:     $Date: 2003/12/27 15:06:40 $
 *            by: bkorb
 */

/* FORWARD */

STATIC void
compressDef( char* pz );

STATIC char*
emitListattr( char* pzText, char* pzOut );

STATIC char*
emitQuote( char** ppzText, char* pzOut );

STATIC char*
emitSubblock( tCC* pzDefList, char* pzText, char* pzOut );

STATIC char*
emitSubblockString( char** ppzText, char sepChar, char* pzOut );
/* END-FORWARD */

/*
 *  compressDef
 *
 *  Compress the definition text.  Each input line has some prefix
 *  stuff to ensure it is a comment as seen by the normal processor
 *  of the file.  In "C", the entire block is surrounded by the
 *  '/'-'*' and '*'-'/' pairs.  In shell, every line would start
 *  with a hash character ('#').  Etc.  To make life easy, we require
 *  that every line be prefixed with something that matches the
 *  pattern:
 *
 *        "^[^*]*\*"
 *
 *  and any line that does not is ignored.  So, here we strip off
 *  that prefix before we go ahead and try to parse it.
 */
STATIC void
compressDef( char* pz )
{
    char* pzStrt = pz;
    char* pzDest = pz;
    char* pzSrc  = pz;
    int   nlCt   = 0;

    /*
     *  Search until we find a line that contains an asterisk
     *  and is followed by something other than whitespace.
     */
    nlCt =  0;
 skip_leading_space:
    while (isspace( *pzSrc )) {
        if (*(pzSrc++) == '\n') {
            nlCt++;
            for (;;) {
                switch (*(pzSrc++)) {
                case '*':
                    while (*pzSrc == '*')     pzSrc++;
                    goto skip_leading_space;

                case NUL:
                    *pzStrt = NUL;
                    return;

                case '\n':
                    nlCt++;
                }
            }
        }
    }

    if (*pzSrc == NUL) {
        *pzStrt = NUL;
        return;
    }

    /*
     *  FOR as long as we still have more text, ...
     */
    for (;;) {
        /*
         *  IF we passed over one or more newlines while looking for
         *  an asterisk, then insert one extra newline into the output
         */
        if (nlCt > 0) {
            *pzDest++ = '\n';
            nlCt =  0;
        }

        /*
         *  FOR all the data on the current input line, ...
         */
        for (;;) {
            /*
             *  Move the source to destination until we find
             *  either a new-line or a NUL.
             */
            switch (*pzDest++ = *pzSrc++) {
            case '\n':
                if (*pzSrc != NUL)
                    goto lineDone;

            case NUL:
                pzDest--;
                goto compressDone;

            default:
                ;
            }
        } lineDone:;

        /*
         *  Trim trailing white space off the end of the line.
         */
        if ((pzDest[-2] == ' ') || (pzDest[-2] == '\t')) {
            do  {
                pzDest--;
            } while ((pzDest[-2] == ' ') || (pzDest[-2] == '\t'));
            pzDest[-1] = '\n';
        }

        /*
         *  We found a new-line.  Skip forward to an asterisk.
         */
    foundNewline:
        while (*pzSrc != '*') {
            if (*pzSrc == NUL)
                goto compressDone;
            if (*pzSrc == '\n')
                nlCt++;
            pzSrc++;
        }

        /*
         *  Skip over the asterisk we found and all the ones that follow
         */
        while (*pzSrc == '*')     pzSrc++;
        while (isspace( *pzSrc )) {
            /*
             *  IF we stumble into another newline,
             *  THEN we go back to look for an asterisk.
             */
            if (*pzSrc == '\n')
                goto foundNewline;
            pzSrc++;
        }
    } compressDone:;

    /*
     *  Trim off all the trailing white space, including newlines
     */
    while ((pzDest > pzStrt) && isspace( pzDest[-1] )) pzDest--;
    *pzDest = NUL;
}


/*
 *  emitDefinition
 */
EXPORT char*
emitDefinition( char* pzDef, char* pzOut )
{
    char   sep_char;
    char   zEntryName[ MAXNAMELEN ];

    /*
     *  Indent attribute definitions four spaces
     */
    {
        char*  p = zEntryName;
        *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' ';

        while (AG_NAME_CHAR(*pzDef))
            *p++ = *pzOut++ = *pzDef++;

        if (p >= zEntryName + sizeof( zEntryName )) {
            fprintf( stderr, "getdefs error:  names constrained to %d bytes\n",
                     MAXNAMELEN );
            exit( EXIT_FAILURE );
        }
        *p = NUL;
    }

    /*
     *  Strip the prefixes from all the definition lines
     *  (viz., the "^.*\*" text, except that it is a shortest match
     *  instead of longest match).  Skip the ':' before starting.
     */
    compressDef( ++pzDef );

    if (HAVE_OPT( SUBBLOCK )) {
        int    ct  = STACKCT_OPT(  SUBBLOCK );
        tCC**  ppz = STACKLST_OPT( SUBBLOCK );

        do  {
            tCC* pz = *ppz++;
            if (strcmp( pz, zEntryName ) == 0)
                return emitSubblock( pz, pzDef, pzOut );
        } while (--ct > 0);
    }

    if (HAVE_OPT( LISTATTR )) {
        int    ct  = STACKCT_OPT(  LISTATTR );
        tCC**  ppz = STACKLST_OPT( LISTATTR );

        do  {
            if (strcmp( *ppz++, zEntryName ) == 0)
                return emitListattr( pzDef, pzOut );
        } while (--ct > 0);
    }

    if (isspace( *pzDef ))
         sep_char = *pzDef++;
    else sep_char = ' ';

    switch (*pzDef) {
    case NUL:
        *pzOut++ = ';'; *pzOut++ = '\n';
        break;

    case '"':
    case '\'':
    case '{':
        /*
         *  Quoted entries or subblocks do their own stringification
         *  sprintf is safe because we are copying strings around
         *  and *always* making the result smaller than the original
         */
        pzOut += sprintf( pzOut, " =%c%s;\n", sep_char, pzDef );
        break;

    default:
        *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = sep_char;
        *pzOut++ = '\'';

        for (;;) {
            switch (*pzOut++ = *pzDef++) {
            case '\\':
                *pzOut++  = '\\';
                break;

            case '\'':
                pzOut[-1] = '\\';
                *pzOut++  = '\'';
                break;

            case NUL:
                goto unquotedDone;
            }
        } unquotedDone:;
        pzOut[-1] = '\''; *pzOut++ = ';'; *pzOut++ = '\n';
        break;
    }
    return pzOut;
}


/*
 *  emitListattr
 */
STATIC char*
emitListattr( char* pzText, char* pzOut )
{
    tSCC  zStart[]  = " = ";
    char  sepChar   = ',';
    int   FirstAttr = 1;

    strcpy( pzOut, zStart );
    pzOut += sizeof( zStart ) - 1;

    /*
     *  See if there is an alternate separator character.
     *  It must be a punctuation character that is not also
     *  a quote character.
     */
    if (ispunct( *pzText ) && (*pzText != '"') && (*pzText != '\''))
        sepChar = *(pzText++);

    /*
     *  Loop for as long as we have text entries
     */
    for (;;) {
        while (isspace( *pzText )) pzText++;
        if (*pzText == NUL) {
            /*
             *  IF there were no definitions, THEN emit an empty one
             */
            if (FirstAttr)
                pzOut -= sizeof( zStart ) - 1;

            *(pzOut++) = ';';
            *(pzOut++) = '\n';
            break;
        }

        if (FirstAttr)
            FirstAttr = 0;
        else
            *(pzOut++) = ',';

        if (*pzText == sepChar) {
            *(pzOut++) = '\''; *(pzOut++) = '\'';
            pzText++;
            continue;           
        }
        pzOut = emitSubblockString( &pzText, sepChar, pzOut );
    }

    return pzOut;
}


/*
 *  The text is quoted, so copy it as is, ensuring that escaped
 *  characters are not used to end the quoted text.
 */
STATIC char*
emitQuote( char** ppzText, char* pzOut )
{
    char*  pzText = *ppzText;
    char   svch   = (*pzOut++ = *pzText++);

    for (;;) {
        switch (*pzOut++ = *pzText++) {

        case '\\':
            if ((*pzOut++ = *pzText++) != NUL)
                break;

        case NUL:
            pzText--;
            pzOut[-1] = svch;
            svch = NUL;
            /* FALLTHROUGH */

        case '"':
        case '\'':
            if (pzOut[-1] == svch)
                goto quoteDone;

            break;
        }
    }

quoteDone:
    *ppzText = pzText;
    return pzOut;
}


/*
 *  emitSubblock
 */
STATIC char*
emitSubblock( tCC* pzDefList, char* pzText, char* pzOut )
{
    tSCC  zStart[]  = " = {";
    tSCC  zAttr[]   = "\n        ";
    tSCC  zEnd[]    = "\n    };\n";
    char  sepChar   = ',';
    int   FirstAttr = 1;

    /*
     *  Advance past subblock name to the entry name list
     */
    pzDefList += strlen( pzDefList ) + 1;
    strcpy( pzOut, zStart );
    pzOut += sizeof( zStart ) - 1;

    /*
     *  See if there is an alternate separator character.
     *  It must be a punctuation character that is not also
     *  a quote character.
     */
    if (ispunct( *pzText ) && (*pzText != '"') && (*pzText != '\''))
        sepChar = *(pzText++);

    /*
     *  Loop for as long as we have text entries and subblock
     *  attribute names, ...
     */
    do  {
        /*
         *  IF the first character is the separator,
         *  THEN this entry is skipped.
         */
        if (*pzText == sepChar) {
            pzText++;
            for (;;) {
                switch (*++pzDefList) {
                case ' ':
                    pzDefList++;
                case NUL:
                    goto def_list_skip_done;
                }
            } def_list_skip_done:;
            continue;
        }

        /*
         *  Skip leading white space in the attribute and check for done.
         */
        while (isspace( *pzText )) pzText++;
        if (*pzText == NUL) {
            /*
             *  IF there were no definitions, THEN emit one anyway
             */
            if (FirstAttr) {
                strcpy( pzOut, zAttr );
                pzOut += sizeof( zAttr );
                for (;;) {
                    *pzOut++ = *pzDefList++;
                    switch (*pzDefList) {
                    case ' ':
                    case NUL:
                        goto single_def_entry_done;
                    }
                } single_def_entry_done:;
                *pzOut++ = ';';
            }
            break;
        }

        /*
         *  Copy out the attribute name
         */
        strcpy( pzOut, zAttr );
        pzOut += sizeof( zAttr )-1;
        FirstAttr = 0;

        for (;;) {
            *pzOut++ = *pzDefList++;
            switch (*pzDefList) {
            case ' ':
                pzDefList++;
            case NUL:
                goto def_name_copied;
            }
        } def_name_copied:;

        /*
         *  IF there are no data for this attribute,
         *  THEN we emit an empty definition.
         */
        if (*pzText == sepChar) {
            *pzOut++ = ';';
            pzText++;
            continue;
        }

        /*
         *  Copy out the assignment operator and emit the string
         */
        *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = ' ';
        pzOut = emitSubblockString( &pzText, sepChar, pzOut );
        *pzOut++ = ';';

    } while (isalpha( *pzDefList ));
    strcpy( pzOut, zEnd );
    return pzOut + sizeof( zEnd ) - 1;
}


/*
 *  Emit a string in a fashion that autogen will be able to
 *  correctly reconstruct it.
 */
STATIC char*
emitSubblockString( char** ppzText, char sepChar, char* pzOut )
{
    char*  pzText  = *ppzText;
    char*  pcComma;
    char*  pcEnd;

    /*
     *  Skip leading space
     */
    while (isspace( *pzText )) pzText++;

    /*
     *  IF the text is already quoted,
     *  THEN call the quoted text emitting routine
     */
    if ((*pzText == '"') || (*pzText == '\'')) {
        *ppzText = pzText;
        return emitQuote( ppzText, pzOut );
    }

    /*
     *  Look for the character that separates this entry text
     *  from the entry text for the next attribute.  Leave 'pcComma'
     *  pointing to the character _before_ the character where we
     *  are to resume our text scan.  (i.e. at the comma, or the
     *  last character in the string)
     */
    pcComma = strchr( pzText, sepChar );
    if (pcComma == (char*)NULL) {
        pcEnd = pzText + strlen( pzText );
        pcComma = pcEnd-1;
    } else {
        pcEnd = pcComma;
    }

    /*
     *  Clean off trailing white space.
     */
    while ((pcEnd > pzText) && isspace( pcEnd[-1] )) pcEnd--;

    /*
     *  Copy the text, surrounded by single quotes
     */
    *pzOut++ = '\'';
    {
        char svch = *pcEnd;
        *pcEnd = NUL;
        for (;;) {
            char ch = *pzText++;
            switch (ch) {
            case '\'':
                *pzOut++ = '\\';
            default:
                *pzOut++ = ch;
                break;
            case NUL:
                goto copyDone;
            }
        } copyDone: ;

        pzText = pcComma+1;
        *pcEnd = svch;
    }

    *pzOut++ = '\'';
    *ppzText = pzText;
    return pzOut;
}

/* emacs
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of getdefs/gdemit.c */
