/*  
 *  EDIT THIS FILE WITH CAUTION  (defParse-fsm.c)
 *  
 *  It has been AutoGen-ed  Saturday January 17, 2004 at 09:56:10 AM PST
 *  From the definitions    defParse.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (c) 2001-2003  by  Bruce Korb
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
#include "defParse-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */

static char*        pz_new_name   = NULL;
static tDefEntry*   pCurrentEntry = NULL;
static int          stackDepth    = 0;
static int          stackSize     = 16;
static tDefEntry*   defStack[16];
static tDefEntry**  ppDefStack    = defStack;

/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

#ifndef tSCC
#  define tSCC static const char
#endif

/*
 *  This is the prototype for the callback routines.  They are to
 *  return the next state.  Normally, that should be the value of
 *  the "maybe_next" argument.
 */
typedef te_dp_state (dp_callback_t)(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt );

static dp_callback_t
    dp_do_have_name_lit_eq,
    dp_do_have_name_lit_semi,
    dp_do_have_value_lit_comma,
    dp_do_indexed_name,
    dp_do_invalid,
    dp_do_need_name_end,
    dp_do_need_name_lit_c_brace,
    dp_do_need_name_var_name,
    dp_do_need_value_lit_o_brace,
    dp_do_str_value,
    dp_do_tpl_name;

/*
 *  This declares all the state transition handling routines
 */
typedef struct transition t_dp_transition;
struct transition {
    te_dp_state      next_state;
    dp_callback_t*   trans_proc;
};

/*
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
static const t_dp_transition
dp_trans_table[ DP_STATE_CT ][ DP_EVENT_CT ] = {
  { { DP_ST_NEED_DEF, NULL },                       /* init state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_def state */
    { DP_ST_NEED_TPL, NULL },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_tpl state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_semi state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_NAME, NULL },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_name state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_DONE, &dp_do_need_name_end },
    { DP_ST_HAVE_NAME, &dp_do_need_name_var_name },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_HAVE_VALUE, &dp_do_need_name_lit_c_brace },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* have_name state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_NAME, &dp_do_have_name_lit_semi },
    { DP_ST_NEED_VALUE, &dp_do_have_name_lit_eq },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_IDX, NULL },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_value state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_HAVE_VALUE, &dp_do_str_value },
    { DP_ST_HAVE_VALUE, &dp_do_str_value },
    { DP_ST_HAVE_VALUE, &dp_do_str_value },
    { DP_ST_HAVE_VALUE, &dp_do_str_value },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_NAME, &dp_do_need_value_lit_o_brace },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_idx state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_CBKT, &dp_do_indexed_name },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_CBKT, &dp_do_indexed_name },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* need_cbkt state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INDX_NAME, NULL } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* indx_name state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_NAME, NULL },
    { DP_ST_NEED_VALUE, NULL },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } },
  { { DP_ST_INVALID, &dp_do_invalid },              /* have_value state */
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_NAME, NULL },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_NEED_VALUE, &dp_do_have_value_lit_comma },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid },
    { DP_ST_INVALID, &dp_do_invalid } }
};


#ifndef HAVE_ZBOGUS
#define HAVE_ZBOGUS
/*
 *  Define all the event and state names, once per compile unit.
 */
tSCC zBogus[]     = "** OUT-OF-RANGE **";
tSCC zFsmErr[]    =
    "FSM Error:  in state %d (%s), event %d (%s) is invalid\n";
#endif /* HAVE_ZBOGUS */
tSCC zDpStInit[]    = "init";
tSCC zDpStNeed_Def[] = "need_def";
tSCC zDpStNeed_Tpl[] = "need_tpl";
tSCC zDpStNeed_Semi[] = "need_semi";
tSCC zDpStNeed_Name[] = "need_name";
tSCC zDpStHave_Name[] = "have_name";
tSCC zDpStNeed_Value[] = "need_value";
tSCC zDpStNeed_Idx[] = "need_idx";
tSCC zDpStNeed_Cbkt[] = "need_cbkt";
tSCC zDpStIndx_Name[] = "indx_name";
tSCC zDpStHave_Value[] = "have_value";
tSCC* apzDpStates[] = {
    zDpStInit,       zDpStNeed_Def,   zDpStNeed_Tpl,   zDpStNeed_Semi,
    zDpStNeed_Name,  zDpStHave_Name,  zDpStNeed_Value, zDpStNeed_Idx,
    zDpStNeed_Cbkt,  zDpStIndx_Name,  zDpStHave_Value };

tSCC zDpEvInvalid[] = "* Invalid Event *";
tSCC zDpEvAutogen[] = "autogen";
tSCC zDpEvDefinitions[] = "definitions";
tSCC zDpEvEnd[] = "End-Of-File";
tSCC zDpEvVar_Name[] = "var_name";
tSCC zDpEvOther_Name[] = "other_name";
tSCC zDpEvString[] = "string";
tSCC zDpEvNumber[] = "number";
tSCC zDpEvLit_Semi[] = ";";
tSCC zDpEvLit_Eq[] = "=";
tSCC zDpEvLit_Comma[] = ",";
tSCC zDpEvLit_O_Brace[] = "{";
tSCC zDpEvLit_C_Brace[] = "}";
tSCC zDpEvLit_Open_Bkt[] = "[";
tSCC zDpEvLit_Close_Bkt[] = "]";
tSCC* apzDpEvents[] = {
    zDpEvAutogen,       zDpEvDefinitions,   zDpEvEnd,
    zDpEvVar_Name,      zDpEvOther_Name,    zDpEvString,
    zDpEvNumber,        zDpEvLit_Semi,      zDpEvLit_Eq,
    zDpEvLit_Comma,     zDpEvLit_O_Brace,   zDpEvLit_C_Brace,
    zDpEvLit_Open_Bkt,  zDpEvLit_Close_Bkt, zDpEvInvalid };

#define DP_EVT_NAME(t) ( (((unsigned)(t)) >= DP_EV_INVALID) \
    ? zBogus : apzDpEvents[ t ])

#define DP_STATE_NAME(s) ( (((unsigned)(s)) > DP_ST_INVALID) \
    ? zBogus : apzDpStates[ s ])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
int
dp_invalid_transition( te_dp_state st, te_dp_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    fprintf( stderr, zFsmErr, st, DP_STATE_NAME(st), evt, DP_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

static te_dp_state
dp_do_have_name_lit_eq(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == HAVE NAME LIT EQ == DO NOT CHANGE THIS COMMENT  */
    tDefEntry* pDE = findPlace( pz_new_name, NULL );
    if (pCurrentEntry != NULL)
        addSibMacro( pDE, pCurrentEntry );
    pCurrentEntry = pDE;
    return maybe_next;
/*  END   == HAVE NAME LIT EQ == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_have_name_lit_semi(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == HAVE NAME LIT SEMI == DO NOT CHANGE THIS COMMENT  */
    tDefEntry* pDE = findPlace( pz_new_name, NULL );
    if (pCurrentEntry != NULL)
        addSibMacro( pDE, pCurrentEntry );
    pCurrentEntry = pDE;
    makeMacro( pCurrentEntry, "", VALTYP_TEXT );
    return maybe_next;
/*  END   == HAVE NAME LIT SEMI == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_have_value_lit_comma(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == HAVE VALUE LIT COMMA == DO NOT CHANGE THIS COMMENT  */
    return maybe_next;
/*  END   == HAVE VALUE LIT COMMA == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_indexed_name(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == INDEXED NAME == DO NOT CHANGE THIS COMMENT  */
    tDefEntry* pDE = findPlace( pz_new_name, pz_token );
    if (pCurrentEntry != NULL)
        addSibMacro( pDE, pCurrentEntry );
    pCurrentEntry = pDE;
    return maybe_next;
/*  END   == INDEXED NAME == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_invalid(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == INVALID == DO NOT CHANGE THIS COMMENT  */
    dp_invalid_transition( initial, trans_evt );
    yyerror( "invalid transition" );
/*  END   == INVALID == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_need_name_end(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == NEED NAME END == DO NOT CHANGE THIS COMMENT  */
    if (stackDepth != 0)
        yyerror( "definition blocks were left open" );
    return maybe_next;
/*  END   == NEED NAME END == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_need_name_lit_c_brace(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == NEED NAME LIT C BRACE == DO NOT CHANGE THIS COMMENT  */
    if (--stackDepth < 0)
        yyerror( "Too many close braces" );

    makeMacroList( ppDefStack[ stackDepth ], pCurrentEntry, VALTYP_BLOCK );
    pCurrentEntry = ppDefStack[ stackDepth ];
    return maybe_next;
/*  END   == NEED NAME LIT C BRACE == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_need_name_var_name(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == NEED NAME VAR NAME == DO NOT CHANGE THIS COMMENT  */
    pz_new_name = pz_token;
    return maybe_next;
/*  END   == NEED NAME VAR NAME == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_need_value_lit_o_brace(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == NEED VALUE LIT O BRACE == DO NOT CHANGE THIS COMMENT  */
    if (pCurrentEntry->valType == VALTYP_TEXT)
        yyerror( "assigning a block value to text name" );

    if (++stackDepth > stackSize) {
        tDefEntry** ppDE;
        stackSize += stackSize / 2;

        if (ppDefStack == defStack) {
            ppDE = AGALOC( stackSize * sizeof(void*), "def stack" );
            memcpy( ppDE, defStack, sizeof( defStack ));
        } else {
            ppDE = AGREALOC( ppDefStack, stackSize * sizeof(void*),
                             "stretch def stack" );
        }
        ppDefStack = ppDE;
    }
    makeMacro( pCurrentEntry, NULL, VALTYP_BLOCK );
    ppDefStack[ stackDepth-1 ] = pCurrentEntry;
    return maybe_next;
/*  END   == NEED VALUE LIT O BRACE == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_str_value(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == STR VALUE == DO NOT CHANGE THIS COMMENT  */
    if (pCurrentEntry->valType == VALTYP_BLOCK)
        yyerror( "assigning a block value to text name" );

    makeMacro( pCurrentEntry, pz_token, VALTYP_TEXT );
    return maybe_next;
/*  END   == STR VALUE == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_tpl_name(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == TPL NAME == DO NOT CHANGE THIS COMMENT  */
    identify( pz_token );
    return maybe_next;
/*  END   == TPL NAME == DO NOT CHANGE THIS COMMENT  */
}

/*
 *  Run the FSM.  Will return DP_ST_DONE or DP_ST_INVALID
 */
te_dp_state
dp_run_fsm( void )
{
    te_dp_state dp_state = DP_ST_INIT;
    te_dp_event trans_evt;
    te_dp_state nxtSt, firstNext;
    dp_callback_t* pT;

    while (dp_state < DP_ST_INVALID) {

        /* START == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */
        trans_evt = yylex();
        /* END   == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */

        if (trans_evt >= DP_EV_INVALID) {
            nxtSt = DP_ST_INVALID;
            pT    = dp_do_invalid;
        } else {
            const t_dp_transition* pTT = dp_trans_table[ dp_state ] + trans_evt;
            nxtSt = firstNext = pTT->next_state;
            pT    = pTT->trans_proc;
        }

#ifdef DEBUG
        printf( "in state %s(%d) step %s(%d) to %s(%d)\n",
            DP_STATE_NAME( dp_state ), dp_state,
            DP_EVT_NAME( trans_evt ), trans_evt,
            DP_STATE_NAME( nxtSt ), nxtSt );
#endif
        nxtSt = (*pT)( dp_state, nxtSt, trans_evt );
#ifdef DEBUG
        if (nxtSt != firstNext)
            printf( "transition code changed destination state to %s(%d)\n",
                DP_STATE_NAME( nxtSt ), nxtSt );
#endif
        dp_state = nxtSt;
    }
    return dp_state;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * tab-width: 4
 * End:
 * end of defParse-fsm.c */
