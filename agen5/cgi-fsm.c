/*  
 *  EDIT THIS FILE WITH CAUTION  (cgi-fsm.c)
 *  
 *  It has been AutoGen-ed  Sunday November 25, 2001 at 03:34:10 PM PST
 *  From the definitions    cgi.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (c) 2001  by  Bruce Korb
 *
 *  AutoFSM is free software copyrighted by Bruce Korb.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name ``Bruce Korb'' nor the name of any other
 *     contributor may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *  
 *  AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM
#include "cgi-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
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
#ifndef DEBUG_FSM
#  undef DEBUG
#endif

/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

#ifndef tSCC
#  define tSCC static const char
#endif

/*
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
    CGI_TR_INIT_ALPHA,
    CGI_TR_INVALID,
    CGI_TR_NAME_EQUAL,
    CGI_TR_SEPERATE,
    CGI_TR_STASH,
    CGI_TR_VALUE_ESCAPE,
    CGI_TR_VALUE_SPACE
} te_cgi_trans;
#define CGI_TRANSITION_CT  7

/*
 *  the state transition handling map
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
typedef struct transition t_transition;
struct transition {
    te_cgi_state  next_state;
    te_cgi_trans  transition;
};
static const t_transition cgi_trans_table[ CGI_STATE_CT ][ CGI_EVENT_CT ] = {
  { { CGI_ST_NAME, CGI_TR_INIT_ALPHA },             /* init state */
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID } },

  { { CGI_ST_NAME, CGI_TR_STASH },                  /* name state */
    { CGI_ST_NAME, CGI_TR_STASH },
    { CGI_ST_VALUE, CGI_TR_NAME_EQUAL },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID },
    { CGI_ST_INVALID, CGI_TR_INVALID } },

  { { CGI_ST_VALUE, CGI_TR_STASH },                 /* value state */
    { CGI_ST_VALUE, CGI_TR_STASH },
    { CGI_ST_VALUE, CGI_TR_STASH },
    { CGI_ST_VALUE, CGI_TR_VALUE_SPACE },
    { CGI_ST_VALUE, CGI_TR_VALUE_ESCAPE },
    { CGI_ST_VALUE, CGI_TR_STASH },
    { CGI_ST_INIT, CGI_TR_SEPERATE },
    { CGI_ST_DONE, CGI_TR_SEPERATE } }
};


/*
 *  Define all the event and state names
 */
tSCC zBogus[]     = "** OUT-OF-RANGE **";
tSCC zStInit[]    = "init";
tSCC zEvInvalid[] = "* Invalid Event *";
tSCC zFsmErr[]    =
    "in state %d (%s), event %d (%s) is invalid\n";

tSCC zStName[] = "name";
tSCC zStValue[] = "value";
tSCC* apzStates[] = {
    zStInit,  zStName,  zStValue };

tSCC zEvAlpha[] = "alpha";
tSCC zEvName_Char[] = "name_char";
tSCC zEvEqual[] = "equal";
tSCC zEvSpace[] = "space";
tSCC zEvEscape[] = "escape";
tSCC zEvOther[] = "other";
tSCC zEvSeperator[] = "seperator";
tSCC zEvEnd[] = "end";
tSCC* apzEvents[] = {
    zEvAlpha,     zEvName_Char, zEvEqual,     zEvSpace,     zEvEscape,
    zEvOther,     zEvSeperator, zEvEnd,       zEvInvalid };

#define CGI_EVT_NAME(t) ( (((unsigned)(t)) >= CGI_EV_INVALID) \
    ? zBogus : apzEvents[ t ])

#define CGI_STATE_NAME(s) ( (((unsigned)(s)) > CGI_ST_INVALID) \
    ? zBogus : apzStates[ s ])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
int
cgi_invalid_transition( te_cgi_state st, te_cgi_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char* pz = asprintf( zFsmErr, st, CGI_STATE_NAME( st ),
						 evt, CGI_EVT_NAME( evt ));
	pz = asprintf( "CGI parsing error:  %s", pz );
    AG_ABEND( pz );
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}


te_cgi_state
cgi_run_fsm( const char* pzSrc, int inlen, char* pzOut, int outlen )
{
    te_cgi_state cgi_state = CGI_ST_INIT;
    te_cgi_event trans_evt;
    te_cgi_state nxtSt, firstNext;
    te_cgi_trans trans;

    while (cgi_state < CGI_ST_INVALID) {

        /* START == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */

        char  curCh;
        if (--inlen < 0)
            trans_evt = CGI_EV_END;
        else {
            if (outlen < 4) {
                strcpy( pzOut, "output space exhausted\n" );
                return CGI_ST_INVALID;
            }
            curCh = *(pzSrc++);
            if (isalpha( curCh ))
                trans_evt = CGI_EV_ALPHA;
            else if (isdigit( curCh ))
                trans_evt = CGI_EV_NAME_CHAR;
            else switch (curCh) {
            case '_': trans_evt = CGI_EV_NAME_CHAR; break;
            case '=': trans_evt = CGI_EV_EQUAL;     break;
            case '+': trans_evt = CGI_EV_SPACE;     break;
            case '%': trans_evt = CGI_EV_ESCAPE;    break;
            case '&': trans_evt = CGI_EV_SEPERATOR; break;
            default:  trans_evt = CGI_EV_OTHER;     break;
            }
        }

        /* END   == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */

        if (trans_evt >= CGI_EV_INVALID) {
            nxtSt = CGI_ST_INVALID;
            trans = CGI_TR_INVALID;
        } else {
            const t_transition* pTT = cgi_trans_table[ cgi_state ] + trans_evt;
            nxtSt = firstNext = pTT->next_state;
            trans = pTT->transition;
        }

#ifdef DEBUG
        printf( "in state %s(%d) step %s(%d) to %s(%d)\n",
            CGI_STATE_NAME( cgi_state ), cgi_state,
            CGI_EVT_NAME( trans_evt ), trans_evt,
            CGI_STATE_NAME( nxtSt ), nxtSt );
#endif

        switch (trans) {
        case CGI_TR_INIT_ALPHA:
            /* START == INIT_ALPHA == DO NOT CHANGE THIS COMMENT */
            *(pzOut++) = curCh;
            outlen--;
            /* END   == INIT_ALPHA == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_INVALID:
            /* START == INVALID == DO NOT CHANGE THIS COMMENT */
            exit( cgi_invalid_transition( cgi_state, trans_evt ));
            /* END   == INVALID == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_NAME_EQUAL:
            /* START == NAME_EQUAL == DO NOT CHANGE THIS COMMENT */
            strcpy( pzOut, "='" );
            outlen -= 2;
            pzOut += 2;
            /* END   == NAME_EQUAL == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_SEPERATE:
            /* START == SEPERATE == DO NOT CHANGE THIS COMMENT */
            strcpy( pzOut, "';\n" );
            outlen -= 2;
            pzOut += 3;
            /* END   == SEPERATE == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_STASH:
            /* START == STASH == DO NOT CHANGE THIS COMMENT */
            *(pzOut++) = curCh;
            outlen--;
            /* END   == STASH == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_VALUE_ESCAPE:
            /* START == VALUE_ESCAPE == DO NOT CHANGE THIS COMMENT */
            {
            char z[4];
            if (inlen < 2)
                exit( cgi_invalid_transition( cgi_state, trans_evt ));

            z[0] = *(pzSrc++);
            z[1] = *(pzSrc++);
            z[2] = '\0';
            inlen -= 2;

            switch (*(pzOut++) = (int)strtol( z, NULL, 16 )) {
            case '\'':
            case '\\':
            case '#':
                pzOut[0]  = pzOut[-1];
                pzOut[-1] = '\\';
                pzOut++;
            }
            }
            /* END   == VALUE_ESCAPE == DO NOT CHANGE THIS COMMENT */
            break;

        case CGI_TR_VALUE_SPACE:
            /* START == VALUE_SPACE == DO NOT CHANGE THIS COMMENT */
            *(pzOut++) = ' ';
            outlen--;
            /* END   == VALUE_SPACE == DO NOT CHANGE THIS COMMENT */
            break;

        default:
            /* START == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
            exit( cgi_invalid_transition( cgi_state, trans_evt ));
            /* END   == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
        }
#ifdef DEBUG
        if (nxtSt != firstNext)
            printf( "transition code changed destination state to %s(%d)\n",
                CGI_STATE_NAME( nxtSt ), nxtSt );
#endif
        cgi_state = nxtSt;
    }
    return cgi_state;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * End:
 * end of cgi-fsm.c */
