/*  
 *  EDIT THIS FILE WITH CAUTION  (defParse-fsm.c)
 *  
 *  It has been AutoGen-ed  Wednesday March  3, 2004 at 08:58:11 PM PST
 *  From the definitions    defParse.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (c) 2001-2004  by  Bruce Korb
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

static char* pz_new_name = NULL;

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
    dp_do_empty_val,
    dp_do_end_block,
    dp_do_have_name_lit_eq,
    dp_do_indexed_name,
    dp_do_invalid,
    dp_do_need_name_end,
    dp_do_need_name_var_name,
    dp_do_next_val,
    dp_do_start_block,
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

  /* STATE 0:  DP_ST_INIT */
  { { DP_ST_NEED_DEF, NULL },                       /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 1:  DP_ST_NEED_DEF */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_NEED_TPL, NULL },                       /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 2:  DP_ST_NEED_TPL */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },           /* EVT:  var_name */
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },           /* EVT:  other_name */
    { DP_ST_NEED_SEMI, &dp_do_tpl_name },           /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 3:  DP_ST_NEED_SEMI */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_NEED_NAME, NULL },                      /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 4:  DP_ST_NEED_NAME */
  { { DP_ST_NEED_DEF, NULL },                       /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_DONE, &dp_do_need_name_end },           /* EVT:  End-Of-File */
    { DP_ST_HAVE_NAME, &dp_do_need_name_var_name }, /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_HAVE_VALUE, &dp_do_end_block },         /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 5:  DP_ST_HAVE_NAME */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_NEED_NAME, &dp_do_empty_val },          /* EVT:  ; */
    { DP_ST_NEED_VALUE, &dp_do_have_name_lit_eq },  /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_NEED_IDX, NULL },                       /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 6:  DP_ST_NEED_VALUE */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_HAVE_VALUE, &dp_do_str_value },         /* EVT:  var_name */
    { DP_ST_HAVE_VALUE, &dp_do_str_value },         /* EVT:  other_name */
    { DP_ST_HAVE_VALUE, &dp_do_str_value },         /* EVT:  string */
    { DP_ST_HAVE_VALUE, &dp_do_str_value },         /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_NEED_NAME, &dp_do_start_block },        /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 7:  DP_ST_NEED_IDX */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_NEED_CBKT, &dp_do_indexed_name },       /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_NEED_CBKT, &dp_do_indexed_name },       /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 8:  DP_ST_NEED_CBKT */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INDX_NAME, NULL }                       /* EVT:  ] */
  },

  /* STATE 9:  DP_ST_INDX_NAME */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_NEED_NAME, &dp_do_empty_val },          /* EVT:  ; */
    { DP_ST_NEED_VALUE, NULL },                     /* EVT:  = */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  },

  /* STATE 10:  DP_ST_HAVE_VALUE */
  { { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  autogen */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  definitions */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  End-Of-File */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  var_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  other_name */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  string */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  number */
    { DP_ST_NEED_NAME, NULL },                      /* EVT:  ; */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  = */
    { DP_ST_NEED_VALUE, &dp_do_next_val },          /* EVT:  , */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  { */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  } */
    { DP_ST_INVALID, &dp_do_invalid },              /* EVT:  [ */
    { DP_ST_INVALID, &dp_do_invalid }               /* EVT:  ] */
  }
};


tSCC zDpStrings[] =
       "** OUT-OF-RANGE **" "\0"
       "FSM Error:  in state %d (%s), event %d (%s) is invalid\n" "\0"
       "invalid" "\0"
       "init" "\0"
       "need_def" "\0"
       "need_tpl" "\0"
       "need_semi" "\0"
       "need_name" "\0"
       "have_name" "\0"
       "need_value" "\0"
       "need_idx" "\0"
       "need_cbkt" "\0"
       "indx_name" "\0"
       "have_value" "\0"
       "autogen" "\0"
       "definitions" "\0"
       "End-Of-File" "\0"
       "var_name" "\0"
       "other_name" "\0"
       "string" "\0"
       "number" "\0"
       ";" "\0"
       "=" "\0"
       "," "\0"
       "{" "\0"
       "}" "\0"
       "[" "\0"
       "]" "\0";

#define DpBogus_off 0
#define DpFsmErr_off 19
#define DpEvInvalid_off 75
#define DpStInit_off 83
#define DpStNeed_Def_off 88
#define DpStNeed_Tpl_off 97
#define DpStNeed_Semi_off 106
#define DpStNeed_Name_off 116
#define DpStHave_Name_off 126
#define DpStNeed_Value_off 136
#define DpStNeed_Idx_off 147
#define DpStNeed_Cbkt_off 156
#define DpStIndx_Name_off 166
#define DpStHave_Value_off 176
#define DpEvAutogen_off 187
#define DpEvDefinitions_off 195
#define DpEvEnd_off 207
#define DpEvVar_Name_off 219
#define DpEvOther_Name_off 228
#define DpEvString_off 239
#define DpEvNumber_off 246
#define DpEvLit_Semi_off 253
#define DpEvLit_Eq_off 255
#define DpEvLit_Comma_off 257
#define DpEvLit_O_Brace_off 259
#define DpEvLit_C_Brace_off 261
#define DpEvLit_Open_Bkt_off 263
#define DpEvLit_Close_Bkt_off 265


static const size_t aszDpStates[] = {
    DpStInit_off,       DpStNeed_Def_off,   DpStNeed_Tpl_off,
    DpStNeed_Semi_off,  DpStNeed_Name_off,  DpStHave_Name_off,
    DpStNeed_Value_off, DpStNeed_Idx_off,   DpStNeed_Cbkt_off,
    DpStIndx_Name_off,  DpStHave_Value_off };

static const size_t aszDpEvents[] = {
    DpEvAutogen_off,       DpEvDefinitions_off,   DpEvEnd_off,
    DpEvVar_Name_off,      DpEvOther_Name_off,    DpEvString_off,
    DpEvNumber_off,        DpEvLit_Semi_off,      DpEvLit_Eq_off,
    DpEvLit_Comma_off,     DpEvLit_O_Brace_off,   DpEvLit_C_Brace_off,
    DpEvLit_Open_Bkt_off,  DpEvLit_Close_Bkt_off, DpEvInvalid_off };

#define DP_EVT_NAME(t) ( (((unsigned)(t)) >= DP_EV_INVALID) \
    ? zDpStrings : zDpStrings + aszDpEvents[t])

#define DP_STATE_NAME(s) ( (((unsigned)(s)) > DP_ST_INVALID) \
    ? zDpStrings : zDpStrings + aszDpStates[s])

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
    fprintf( stderr, zDpStrings + DpFsmErr_off, st, DP_STATE_NAME(st),
             evt, DP_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

static te_dp_state
dp_do_empty_val(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == EMPTY VAL == DO NOT CHANGE THIS COMMENT  */
    /*
     *  Our state is either "have-name" or "indx-name" and we found a ';',
     *  end of statement.  It is a string value with an empty string.
     */
    tDefEntry* pDE = findPlace( pz_new_name, NULL );
    pDE->val.pzText = (char*)zNil;
    pDE->valType    = VALTYP_TEXT;
    return maybe_next;
/*  END   == EMPTY VAL == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_end_block(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == END BLOCK == DO NOT CHANGE THIS COMMENT  */
    if (stackDepth <= 0)
        yyerror( "Too many close braces" );

    pCurrentEntry = ppParseStack[ stackDepth-- ];
    return maybe_next;
/*  END   == END BLOCK == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_have_name_lit_eq(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == HAVE NAME LIT EQ == DO NOT CHANGE THIS COMMENT  */
    /*
     *  Create a new entry but defer "makeMacro" call until we have the
     *  assigned value.
     */
    findPlace( pz_new_name, NULL );
    return maybe_next;
/*  END   == HAVE NAME LIT EQ == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_indexed_name(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == INDEXED NAME == DO NOT CHANGE THIS COMMENT  */
    /*
     *  Create a new entry with a specified indes, but defer "makeMacro" call
     *  until we have the assigned value.
     */
    findPlace( pz_new_name, pz_token );
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
    /* NOTREACHED */
    return DP_ST_INVALID;
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

    /*
     *  We won't be using the parse stack any more.
     *  We only process definitions once.
     */
    if (ppParseStack != parseStack)
        AGFREE( ppParseStack );

    /*
     *  The seed has now done its job.  The real root of the
     *  definitions is the first entry off of the seed.
     */
    rootDefCtx.pDefs = rootDefCtx.pDefs->val.pDefEntry;
    return maybe_next;
/*  END   == NEED NAME END == DO NOT CHANGE THIS COMMENT  */
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
dp_do_next_val(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == NEXT VAL == DO NOT CHANGE THIS COMMENT  */
    /*
     *  Clone the entry name of the current entry.
     */
    findPlace( pCurrentEntry->pzDefName, NULL );
    return maybe_next;
/*  END   == NEXT VAL == DO NOT CHANGE THIS COMMENT  */
}

static te_dp_state
dp_do_start_block(
    te_dp_state initial,
    te_dp_state maybe_next,
    te_dp_event trans_evt )
{
/*  START == START BLOCK == DO NOT CHANGE THIS COMMENT  */
    if (pCurrentEntry->valType == VALTYP_TEXT)
        yyerror( "assigning a block value to text name" );
    pCurrentEntry->valType = VALTYP_BLOCK; /* in case not yet determined */

    if (++stackDepth >= stackSize) {
        tDefEntry** ppDE;
        stackSize += stackSize / 2;

        if (ppParseStack == parseStack) {
            ppDE = AGALOC( stackSize * sizeof(void*), "def stack" );
            memcpy( ppDE, parseStack, sizeof( parseStack ));
        } else {
            ppDE = AGREALOC( ppParseStack, stackSize * sizeof(void*),
                             "stretch def stack" );
        }
        ppParseStack = ppDE;
    }
    ppParseStack[ stackDepth ] = pCurrentEntry;
    pCurrentEntry = NULL;
    return maybe_next;
/*  END   == START BLOCK == DO NOT CHANGE THIS COMMENT  */
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

    pCurrentEntry->val.pzText = pz_token;
    pCurrentEntry->valType = VALTYP_TEXT;
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
    /*
     *  Allow this routine to be called multiple times.
     *  This may happen if we include another definition file.
     */
    if (rootDefCtx.pDefs == NULL) {
        tSCC   zBogus[] = "@BOGUS@";
        static tDefEntry seed = {
            NULL, NULL, NULL, NULL, (char*)zBogus, 0, { NULL },
            (char*)zBogus, 0, VALTYP_BLOCK };

        rootDefCtx.pDefs = &seed;

        if (! HAVE_OPT(OVERRIDE_TPL))
             pzTemplFileName = pz_token;

        stackDepth = 0;
        ppParseStack[0] = &seed;
    }
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
        if (pT != NULL)
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
/* end of defParse-fsm.c */
