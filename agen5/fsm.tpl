[= AutoGen5 Template  -*-  Mode:  C  -*-

x

=]
[= (dne " *  " "/*  ") =]
 *
 *  Automated Finite State Machine
 *
 *  This file describes a finite state machine.  It consists of three
 *  enumerations:
 *
 *  1.  The available states.  FSS_INIT is always defined to be zero
 *      and FSS_INVALID and FSS_DONE are always made the last entries.
 *  2.  The transition tokens.  These enumerate the token values used
 *      to select the next state from the current state.
 *      FST_INVALID is always defined at the end.
 *  3.  The list of valid state + transition token names.  Only the
 *      transitions defined to be valid are enumerated.
 *
 *  Plus a transition table indexed by current state and then the
 *  transition token enumeration.  The table contains both the next
 *  state *and* the appropriate state/transition enumeration.
 *
 *  If you AutoGen this with "EXAMPLE" defined in your environment,
 *  this template will also produce example code for your FSM.
 *
[=(bsd "AutoFSM" "Bruce Korb" " *  ")=]
 *
 *  Here are the available states.
 */
typedef enum {
[=
(out-push-new ".fsm.state")

  =]FSS_INIT = 0
[=

FOR  state

  =]FSS_[= (string-upcase! (get "state")) =]
[=
ENDFOR

  =]FSS_INVALID
FSS_DONE[=

(out-pop) =][=

`columns --spread=3 -I4 -S, < .fsm.state`  =]
} te_fsm_state;[=


FOR token

  =][=

  (shellf
     "tkn=\"%s\" ; eval INIT_${tkn}=\\\"{ FSS_INVALID, FSX_INVALID }\\\""
     (string-upcase! (get "token")))  =][=

  FOR state =][=

    (shellf "eval %s_${tkn}=\\\"{ FSS_INVALID, FSX_INVALID }\\\""
            (string-upcase! (get "state")))   =][=

  ENDFOR

  =][=

ENDFOR

=]

/*
 *  The transition tokens.
 */
typedef enum {
[=
(out-push-new ".fsm.tkn")

  =][=

FOR  token  "\n"

  =]FST_[= (string-upcase! (get "token")) =][=

ENDFOR    =]
FST_INVALID[=

(out-pop) =][=

`columns --spread=3 -I4 -S, < .fsm.tkn`  =]
} te_fsm_token;

/*
 *  This enumerates all the valid state + token-name transitions.
 *  It is actually possible to have multiple specifications for a single
 *  pair, but we unique sort the list and eliminate dups that way.
 *  The last one specified will be the only one effective.
 */
typedef enum {[=
(shellf "state_list=\"INIT %s\""
       (string-upcase! (join " " (stack "state"))))
(define next-state "")
(out-push-new ".fsm.trans") =][=

FOR transition              =][=

  CASE t_st                 =][=
  == "*"                    =]
[=
    (set! next-state (if (exist? "next") (get "next") "${f}"))

    (shellf "for f in ${state_list} ; do
     eval ${f}_%1$s=\\\"{ FSS_%2$s, FSX_${f}_%1$s }\\\"
     echo \"    FSX_${f}_%1$s,\"
     done" (string-upcase! (get "ttkn")) next-state)
  =][=


  ~  [a-z][a-z0-9_]*    =][=

  (shellf "%1$s_%2$s=\"{ FSS_%3$s, FSX_%1$s_%2$s }\""
          (string-upcase! (get "t_st"))
          (string-upcase! (get "ttkn"))
          (string-upcase! (get "next")) )
  =]
    FSX_[=(string-upcase! (get "t_st"))
      =]_[=(string-upcase! (get "ttkn")) =],[=

  *      =][= (error % t_st "Invalid state name: '%s'") =][=

  ESAC   =][=

ENDFOR   =][=

(out-pop)
(shell "sort -u .fsm.trans > .fsm.xlist ; cat .fsm.xlist")
=]
    FSX_INVALID
}  te_transition;

typedef struct transition t_transition;
struct transition {
    te_fsm_state     next_state;
    te_transition    trans_type;
};

/*
 *  This table maps the state enumeration + the token enumeration to
 *  the transition enumeration code and the new state.
 *  It is indexed by first the current state and then the token code.
 *  There are no transition entries defined for the INVALID or DONE
 *  states, obviously  :-).
 */
static t_transition trans_table[ [=
   (+ 1 (count "state")) =] ][ [=(count "token")=] ] = {
[=
(out-push-new ".fsm.ttbl1") =][=

FOR token "\n" =][=
  (shellf "echo $INIT_%s" (string-upcase! (get "token")))  =][=
ENDFOR

=]
[=

(out-pop)
(shell "columns --spread=3 --first-indent=\"  {\" -I4 -S, < .fsm.ttbl1") =] },
[=
FOR state ",\n" =]
[=
  (out-push-new ".fsm.ttbl2")
  (shellf "state=%s" (string-upcase! (get "state"))) =][=

  FOR token =][=
    (shellf "eval echo \\$${state}_%s" (string-upcase! (get "token"))) =]
[=ENDFOR=][=

  (out-pop)
  (shell "columns --spread=3 --first-indent=\"  {\" -I4 -S, < .fsm.ttbl2") =] }[=
ENDFOR =]
};

#ifndef tSCC
#  define tSCC static const char
#endif

    int
fsm_invalid_transition( te_fsm_state old_state, te_fsm_token tkn )
{
    tSCC zBogus[]   = "** OUT-OF-RANGE **";
    tSCC zInvalid[] = "* Invalid Token *";
    tSCC zStInit[]  = "init";
    tSCC zFsmErr[]  =
        "FSM Error:  in state %d (%s), token %d (%s) is invalid\n";
[=
FOR state
=]
    tSCC zSt[=(string-capitalize! (get "state"))=][] = [=
        (c-string (string-downcase! (get "state")))=];[=
ENDFOR

=]
    tSCC* apzStates[] = {
[= (out-push-new ".fsm.name")
=]zStInit,
[=

FOR state ",\n"

=]zSt[=(string-capitalize! (get "state"))=][=

ENDFOR

=][=

(out-pop)
(shell "columns --spread=3 -I8 < .fsm.name")

=] };
[=

FOR token =]
    tSCC zTk[=(string-capitalize! (get "token"))=][] = [=
       (c-string (string-downcase! (get "token")))=];[=
ENDFOR

=]
    tSCC* apzTokens[] = {
[= (out-push-new ".fsm.name") =][=

FOR token ",\n"

=]zTk[=(string-capitalize! (get "token"))=][=

ENDFOR =],
zInvalid[=

(out-pop)
(shell "columns --spread=3 -I8 < .fsm.name")

=] };

    const char* pzState;
    const char* pzToken;

    if ((unsigned)old_state >= FSS_INVALID)
         pzState = zBogus;
    else pzState = apzStates[ old_state ];

    if ((unsigned)tkn > FST_INVALID)
         pzToken = zBogus;
    else pzToken = apzTokens[ tkn ];

    fprintf( stderr, zFsmErr, old_state, pzState, tkn, pzToken );
    return EXIT_FAILURE;
}[=

IF (getenv "EXAMPLE") =]

#ifdef EXAMPLE
/*
 *  You will need to fill in the capitalized pseudo-calls with
 *  code that makes sense for your app.
 */
{
    te_fsm_state fsm_state = FSS_INIT;
    FSM_INITIALIZATIONS();

    while (fsm_state != FSS_DONE) {
        te_fsm_token  fsm_tkn = GET_FSM_TOKEN();
        te_fsm_state  nxt_state;
        te_transition trans;

        /*
         *  IF you are certain fsm_tkn cannot be invalid, you may skip test.
         */
        if (fsm_tkn >= FST_INVALID) {
            nxt_state = FSS_INVALID;
            trans     = FSX_INVALID;
        } else {
            nxt_state = trans_table[ fsm_state ][ fsm_tkn ].next_state;
            trans     = trans_table[ fsm_state ][ fsm_tkn ].trans_type;
        }

        /*
         *  There are only so many "FSX_<state-name>_<token-name>"
         *  transitions that are legal.  See which one we got.
         *  It is legal to alter "nxt_state" while processing these.
         */
        switch (trans) {
[=`

for f in \`cat .fsm.xlist\`
do
  echo "${f}" | sed -e 's,FSX_,        case FSX_,'   -e 's/,/:/'
  echo "${f}" | sed -e 's,FSX_,            HANDLE_,'  -e 's/,/();/'
  echo "            break;"
  echo
done `=]

        case FSX_INVALID:
            fsm_invalid_transition( fsm_state, fsm_tkn );
            exit( EXIT_FAILURE );
        }

        fsm_state = nxt_state;
    }
}
#endif /* EXAMPLE */[=

ENDIF  =][=

`rm -f .fsm.* >&2`=]
