
/*
 *  $Id: loadPseudo.c,v 1.13 2001/07/13 04:23:55 bkorb Exp $
 *
 *  This module processes the "pseudo" macro
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "autogen.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  loadPseudoMacro
 *
 *  Find the start and end macro markers.  In btween we must find the
 *  "autogen" and "template" keywords, followed by any suffix specs.
 */
#define DEFINE_FSM
#include "pseudo-fsm.h"

tSCC zAgName[] = "autogen5";
tSCC zTpName[] = "template";


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  skipSuffixSpec
 *
 *  Process a suffix specification
 */
    EXPORT tCC*
doSuffixSpec( tCC* pzData, tCC* pzFileName, int lineNo )
{
    /*
     *  The following is the complete list of POSIX required-to-be-legal
     *  file name characters.  These are the only characters we allow to
     *  appear in a suffix.  We do, however, add '=' and '%' because we
     *  also allow a format specification to follow the suffix,
     *  separated by an '=' character.
     */
    tSCC zSuffixSpecChars[] = "abcdefghijklmnopqrstuvwxyz"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" "-_./" "=%";

    tOutSpec*  pOS;
    char*      pz;
    static tOutSpec**  ppOSList = &pOutSpecList;

    /*
     *  Skip over the suffix construct
     */
    size_t spn = strspn( pzData, zSuffixSpecChars );

    /*
     *  If pzFileName is NULL, then we are called by --select-suffix.
     *  Otherwise, the suffix construct is saved only for the main template,
     *  and only when the --select-suffix option was not specified.
     */
    if (  (pzFileName != NULL)
       && (  (procState != PROC_STATE_LOAD_TPL)
          || HAVE_OPT( SELECT_SUFFIX )))
        return pzData + spn;

    /*
     *  Allocate the suffix structure
     */
    pOS = (tOutSpec*)AGALOC( sizeof( *pOS ) + (size_t)spn + 1,
                             "Output Specification" );

    /*
     *  Link it into the global list
     */
    *ppOSList  = pOS;
    ppOSList   = &pOS->pNext;
    pOS->pNext = (tOutSpec*)NULL;

    /*
     *  Copy the data into the suffix field from our input buffer.
     *  IF the suffix contains its own formatting construct,
     *  THEN split it off from the suffix and set the formatting ptr.
     *  ELSE supply a default.
     */
    strncpy( pOS->zSuffix, pzData, spn );
    pOS->zSuffix[ spn ] = NUL;

    pz = strchr( pOS->zSuffix, '=' );

    if (pz != (char*)NULL) {
        tSCC zFileFmt3[] = "%s";
        *pz++ = NUL;
        if (*pz == NUL)
             pOS->pzFileFmt = zFileFmt3;
        else pOS->pzFileFmt = pz;

    } else {
        tSCC zFileFmt1[] = "%s.%s";
        tSCC zFileFmt2[] = "%s%s";

        /*
         *  IF the suffix does not start with punctuation,
         *  THEN we will insert a '.' of our own.
         */
        if (isalnum( pOS->zSuffix[0] ))
             pOS->pzFileFmt = zFileFmt1;
        else pOS->pzFileFmt = zFileFmt2;
    }

    return pzData + spn;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  findTokenType
 *
 *  Skiping leading white space, figure out what sort of token is under
 *  the scan pointer (pzData).
 */
    STATIC te_pm_event
findTokenType( tCC**  ppzData, te_pm_state fsm_state, ag_bool line_start )
{
    tCC* pzData = *ppzData;
    te_pm_event res_tkn;

 skipWhiteSpace:
    while (isspace( *pzData )) {
        if (*(pzData++) == '\n') {
            line_start = AG_TRUE;
            templLineNo++;

            /*
             *  IF we are done with the macro markers,
             *  THEN we skip white space only thru the first new line.
             */
            if (fsm_state == PM_ST_END_MARK) {
                *ppzData = pzData;
                return PM_EV_END_PSEUDO;
            }
        }
    }

    /*
     *  After the end marker has been found,
     *  anything else is really the start of the data.
     */
    if (fsm_state == PM_ST_END_MARK) {
        *ppzData = pzData;
        return PM_EV_END_PSEUDO;
    }

    /*
     *  IF the token starts with an alphanumeric,
     *  THEN it must be "autogen5" or "template" or a suffix specification
     */
    if (isalnum( *pzData )) {
        if (strneqvcmp( pzData, zAgName, sizeof(zAgName)-1 ) == 0) {
            res_tkn = isspace( pzData[ sizeof(zAgName)-1 ])
                ? PM_EV_AUTOGEN : PM_EV_SUFFIX;

        } else if (strneqvcmp( pzData, zTpName, sizeof(zTpName)-1 ) == 0) {
            res_tkn = isspace( pzData[ sizeof(zAgName)-1 ])
                ? PM_EV_TEMPLATE : PM_EV_SUFFIX;

        } else
            res_tkn = PM_EV_SUFFIX;
    }

    /*
     *  IF the character is '#' and we are at the start of a line,
     *  THEN the entire line is white space.  Skip it.
     */
    else if ((*pzData == '#') && line_start) {
        pzData = strchr( pzData+1, '\n' );
        if (pzData == (char*)NULL)
            return PM_EV_INVALID;

        goto skipWhiteSpace;

    }

    /*
     *  IF the next sequence is "-*-"
     *  THEN we found an edit mode marker
     */
    else if (strncmp( pzData, "-*-", 3 ) == 0)
        res_tkn = PM_EV_ED_MODE;

    /*
     *  IF we are accepting suffixes at this point, then we will
     *  accept these three characters as the start of a suffix
     */
    else if (  (fsm_state == PM_ST_TEMPL)
            && (  (*pzData == '.')
               || (*pzData == '-')
               || (*pzData == '_')
            )  )
        res_tkn = PM_EV_SUFFIX;
    /*
     *  IF it is some other punctuation,
     *  THEN it must be a start/end marker.
     */
    else if (ispunct( *pzData ))
        res_tkn = PM_EV_MARKER;

    /*
     *  Otherwise, it is just junk.
     */
    else
        res_tkn = PM_EV_INVALID;

    *ppzData = pzData;
    return res_tkn;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  copyMarker
 *
 *  Some sort of marker is under the scan pointer.  Copy it for as long
 *  as we find punctuation characters.
 */
    STATIC tCC*
copyMarker( tCC* pzData, char* pzMark, int* pCt )
{
    int ct = 0;

    for (;;) {
        char ch = *pzData;
        if (! ispunct( ch ))
            break;
        *(pzMark++) = ch;
        if (++ct >= sizeof( zStartMac ))
            return (tCC*)NULL;

        pzData++;
    }

    *pCt = ct;
    *pzMark = NUL;
    return pzData;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  loadPseudoMacro
 *
 *  Using a finite state machine, scan over the tokens that make up the
 *  "pseudo macro" at the start of every template.
 */
    EXPORT tCC*
loadPseudoMacro( tCC* pzData, tCC* pzFileName )
{
    tSCC zMarkErr[] = "start/end macro mark too long";
#   define BAD_MARKER( t ) STMTS( \
        fprintf( stderr, zTplErr, pzFileName, templLineNo, t ); AG_ABEND; )

    te_pm_state fsm_state  = PM_ST_INIT;
    ag_bool      line_start = AG_TRUE;  /* set TRUE first time only */

    templLineNo  = 1;

    while (fsm_state != PM_ST_DONE) {
        te_pm_event fsm_tkn = findTokenType( &pzData, fsm_state, line_start );
        te_pm_state nxt_state;
        te_pm_trans trans;

        line_start = AG_FALSE;
        nxt_state  = pm_trans_table[ fsm_state ][ fsm_tkn ].next_state;
        trans      = pm_trans_table[ fsm_state ][ fsm_tkn ].transition;

        /*
         *  There are only so many "PM_TR_<state-name>_<token-name>"
         *  transitions that are legal.  See which one we got.
         *  It is legal to alter "nxt_state" while processing these.
         */
        switch (trans) {
        case PM_TR_AGEN_ED_MODE:
        case PM_TR_INIT_ED_MODE:
        case PM_TR_ST_MARK_ED_MODE:
        case PM_TR_TEMPL_ED_MODE:
        {
            char* pzEnd = strstr( pzData + 3, "-*-" );
            char* pzNL  = strchr( pzData + 3, '\n' );
            if ((pzEnd == NULL) || (pzNL < pzEnd))
                BAD_MARKER( "invalid edit mode marker" );

            pzData = pzEnd + 3;
            break;
        }

        case PM_TR_AGEN_TEMPLATE:
            pzData += sizeof( zTpName )-1;
            break;

        case PM_TR_INIT_MARKER:
            pzData = copyMarker( pzData, zStartMac, &startMacLen );
            if (pzData == (tCC*)NULL)
                BAD_MARKER( zMarkErr );

            break;

        case PM_TR_ST_MARK_AUTOGEN:
            pzData += sizeof( zAgName )-1;
            break;

        case PM_TR_TEMPL_MARKER:
            pzData = copyMarker( pzData, zEndMac, &endMacLen );
            if (pzData == (tCC*)NULL)
                BAD_MARKER( zMarkErr );

            /*
             *  IF the end macro seems to end with the start macro and
             *  it is exactly twice as long as the start macro, then
             *  presume that someone ran the two markers together.
             */
            if (  (endMacLen == 2 * startMacLen)
               && (strcmp( zStartMac, zEndMac + startMacLen ) == 0))  {
                pzData -= startMacLen;
                zEndMac[ startMacLen ] = NUL;
                endMacLen = startMacLen;
            }

            if (strstr( zEndMac, zStartMac ) != NULL)
                BAD_MARKER( "start marker contained in end marker" );
            if (strstr( zStartMac, zEndMac ) != NULL)
                BAD_MARKER( "end marker contained in start marker" );
            break;

        case PM_TR_TEMPL_SUFFIX:
            pzData = doSuffixSpec( pzData, pzFileName, templLineNo );
            break;

        case PM_TR_END_MARK_ED_MODE:
        case PM_TR_INVALID:
            pm_invalid_transition( fsm_state, fsm_tkn );
            switch (fsm_state) {
            case PM_ST_INIT:     BAD_MARKER( "need start marker" );
            case PM_ST_ST_MARK:  BAD_MARKER( "need autogen5 marker" );
            case PM_ST_AGEN:     BAD_MARKER( "need template marker" );
            case PM_ST_TEMPL:    BAD_MARKER( "need end marker" );
            case PM_ST_END_MARK: BAD_MARKER( "need end of line" );
            default:             BAD_MARKER( "BROKEN FSM" );
            }

        case PM_TR_END_MARK_END_PSEUDO:
            /* we be done now */;
        }

        fsm_state = nxt_state;
    }

    return pzData;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of loadPseudo.c */
