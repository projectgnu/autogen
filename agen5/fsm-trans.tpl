[= AutoGen5 Template  -*- Mode: Text -*-=]
[=
(define fmt "")

;;; # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE state-table  =]
  { [=
  (shellf "state=%s" (string-upcase! (get "state"))) =][=

  FOR event "\n    "  =][=
    IF (first-for?)   =][=
       (set! fmt (shellf "eval echo \\\"\\$FSM_TRANS_${state}_%s,\\\""
                 (string-upcase! (get "event"))  ))
       (sprintf "%-47s /* %s state */" fmt (get "state")) =][=
    ELSE     =][=
       (shellf "eval echo \\\"\\$FSM_TRANS_${state}_%s%s\\\""
                 (string-upcase! (get "event"))
                 (if (not (last-for?)) "," "")) =][=
    ENDIF    =][=
  ENDFOR     =] }[=
ENDDEF       =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE  enumerate-transitions   =]
/*
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
[=(string-upcase! (shellf
 "sed '$s/,$//;s/^/    %s_TR_/' .fsm.xlist" PFX))=]
} te_[=(. pfx)=]_trans;
#define [=(. PFX)=]_TRANSITION_CT  [=
   `tct="\`wc -l < .fsm.xlist\`"
   echo $tct`=]

/*
 *  the state transition handling map
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
typedef struct transition t_transition;
struct transition {
    te_[=(. pfx)=]_state  next_state;
    te_[=(. pfx)=]_trans  transition;
};
[=

  IF (exist? "use_ifdef")

=]
#ifndef DEFINE_FSM
extern const t_transition [=(. pfx)=]_trans_table[ [=(. PFX)
=]_STATE_CT ][ [=(. PFX)=]_EVENT_CT ];

extern int
[=(. pfx)=]_invalid_transition( te_[=(. pfx)=]_state st, te_[=
  (. pfx)=]_event evt );
#else
[=

  ELSE

=]static [=
  ENDIF

=]const t_transition [=(. pfx)=]_trans_table[ [=(. PFX)
=]_STATE_CT ][ [=(. PFX)=]_EVENT_CT ] = {[=
  state-table
    state = init =][=

  FOR state      =],
[=  state-table  =][=
  ENDFOR         =]
};[=

  IF (exist? "use_ifdef") =][=
  emit-invalid-msg =]
#endif /* DEFINE_FSM */[=
  ENDIF      =][=

ENDDEF       =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE callback-transitions

=]
/*
 *  This is the prototype for the callback routines.  They are to
 *  return the next state.  Normally, that should be the value of
 *  the "maybe_next" argument.
 */
typedef te_[=(. pfx)=]_state ([=(. pfx)=]_callback_t)([=
    emit-cookie-args =]
    te_[=(. pfx)=]_state initial,
    te_[=(. pfx)=]_state maybe_next,
    te_[=(. pfx)=]_event input_evt );

static [=(. pfx)=]_callback_t
[=(shellf "sed '$s/,$/;/;s/^/    %s_do_/' .fsm.xlist" pfx)=]

/*
 *  This declares all the state transition handling routines
 */
typedef struct transition t_transition;
struct transition {[=
    (set! fmt (sprintf "\n    %%-%ds %%s;"
                (+ (string-length pfx) 14) ))
    (sprintf (string-append fmt fmt)
             (string-append "te_" pfx "_state") "next_state"
             (string-append pfx "_callback_t*") "trans_proc") =]
};

/*
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
static const t_transition trans_table[ [=(. PFX)
=]_STATE_CT ][ [=(. PFX)=]_EVENT_CT ] = {[=

  state-table
    state = init =][=

  FOR state      =],[=
    state-table  =][=
  ENDFOR         =]
};[=

ENDDEF       =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE machine-step  =][=


  IF (=* (get "method") "case")  =]
    if (trans_evt >= [=(. PFX)=]_EV_INVALID) {
        nxtSt = [=(. PFX)=]_ST_INVALID;
        trans = [=(. PFX)=]_TR_INVALID;
    } else {
        const t_transition* pTT = trans_table[ [=(. pfx)=]_state ] + trans_evt;
        nxtSt = firstNext = pTT->next_state;
        trans = pTT->transition;
    }
[=

  ELSE       =]
    if (trans_evt >= [=(. PFX)=]_EV_INVALID) {
        nxtSt = [=(. PFX)=]_ST_INVALID;
        pT    = [=(. pfx)=]_do_invalid;
    } else {
        t_transition* pTT = trans_table[ [=(. pfx)=]_state ] + trans_evt;
        nxtSt = firstNext = pTT->next_state;
        pT    = pTT->trans_proc;
    }
[=

  ENDIF      =]
#ifdef DEBUG
    printf( "in state %s(%d) step %s(%d) to %s(%d)\n",
            [=(. PFX)=]_STATE_NAME( [=(. pfx)=]_state ), [=(. pfx)=]_state,
            [=(. PFX)=]_EVT_NAME( trans_evt ), trans_evt,
            [=(. PFX)=]_STATE_NAME( nxtSt ), nxtSt );
#endif[=


  IF (=* (get "method") "case")  =][=
    run-switch    =][=
  ELSE            =][=
    run-callback  =][=
  ENDIF           =]
[=
ENDDEF       =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE fsm-proc-variables

  =]
    te_[=(. pfx)=]_event trans_evt;
    te_[=(. pfx)=]_state nxtSt, firstNext;[=
    IF (=* (get "method") "call")  =]
    [=(. pfx)=]_callback_t* pT;[=
    ELSE  =]
    te_[=(. pfx)=]_trans trans;[=
    ENDIF =][=

ENDDEF       =][=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE looping-machine

  =]

te_[=(. pfx)=]_state
[=(. pfx)=]_run_fsm( [=
  IF (exist? "cookie") =][=
    FOR cookie ", " =][=cookie=][=ENDFOR=][=
  ELSE=]void[=ENDIF=] )
{
    te_[=(. pfx)=]_state [=(. pfx)=]_state = [=(. PFX)=]_ST_INIT;[=
    fsm-proc-variables  =]

    while ([=(. pfx)=]_state < [=(. PFX)=]_ST_INVALID) {

[=(extract fsm-source "        /* %s == FIND TRANSITION == %s */" ""
           "        trans_evt = GET_NEXT_TRANS();" )=]
[=  (out-push-new ".fsm.cktbl")=][=
    machine-step =][=
    (out-pop)
    (shell "sed 's/^ /     /;s/                /            /' .fsm.cktbl") =]
    }
    return [=(. pfx)=]_state;
}[=

ENDDEF       =][=


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # =][=

DEFINE stepping-machine

  =]

te_[=(. pfx)=]_state
[=(. pfx)=]_step( te_[=(. pfx)=]_event trans_evt[=
   % cookie[0] ", %s"=] )
{[=
    fsm-proc-variables  =]

    if ((unsigned)[=(. pfx)=]_state >= [=(. PFX)=]_ST_INVALID) {
        [=(. pfx)=]_state = [=(. PFX)=]_ST_INIT;
        return [=(. PFX)=]_ST_INVALID;
    }
[=  machine-step =]

[=(extract fsm-source "    /* %s == FINISH STEP == %s */")=]

    return [=(. pfx)=]_state;
}[=


ENDDEF       =]
