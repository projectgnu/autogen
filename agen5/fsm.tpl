[= AutoGen5 Template  -*-  Mode:  text  -*-

h=%s-fsm.h

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
 */[=

  (define hdrname (out-name))
  (define guard   (string-append
          (string-upcase! (string->c-name! (out-name))) "_GUARD" ))

=]
#ifndef [=(. guard)=]
#define [=(. guard)=]
/*
 *  Here are the available states.
 */
typedef enum {
[=
(shellf "columns --spread=1 -I4 -S, -f'FSS_%%s' <<_EOF_
INIT
%s
INVALID
DONE
_EOF_" (string-upcase! (join "\n" (stack "state"))) )=]
} te_fsm_state;

/*
 *  Count of non-terminal states.  FSS_INVALID and FSS_DONE are terminal,
 *  FSS_INIT is not  :-).
 */
#define FSM_STATE_COUNT  [=(+ 1 (count "state"))=]

/*
 *  The transition tokens.
 */
typedef enum {
[=
(shellf "columns --spread=1 -I4 -S, -f'FST_%%s' <<_EOF_
%s
INVALID
_EOF_" (string-upcase! (join "\n" (stack "token"))) )=]
} te_fsm_token;
[=

;;  Initialize every possible transition as invalid
;;
(shellf
  "tk_list='%s' ; st_list='INIT %s'
   for f in $tk_list ; do for g in $st_list
   do  eval FSM_TRANS_${g}_${f}=\"'{ FSS_INVALID, FSX_INVALID }'\"
       export FSM_TRANS_${g}_${f}
   done ; done"
        (string-upcase! (join " " (stack "token")))
        (string-upcase! (join " " (stack "state"))))

(define ttkn  "")
(define tst   "")
(define ttype "")
(define next  "")

;;  Now, evaluate each transition definition and replace the
;;  specified transitions with the defined value.
;;
=][=

FOR transition                 =][=

  IF (== (get "t_st") "*")     =][=

     ;;  This transition applies for all states
     ;;
     (set! ttkn  (string-upcase! (get "ttkn")))
     (set! ttype (if (exist? "ttype") (string-upcase! (get "ttype"))
                 (string-append "${f}_" ttkn) ))
     (set! next  (if (exist? "next") (string-upcase! (get "next")) "${f}"))

     (shellf
       "for f in ${st_list} ; do
       eval FSM_TRANS_${f}_%s=\"'{ FSS_%s, FSX_%s }'\"
       done"
       ttkn next ttype )       =][=

   ELIF (== (get "ttkn") "*")  =][=

     ;;  This transition applies for all transitions in a certain state
     ;;
     (set! tst   (string-upcase! (get "t_st")))
     (set! ttype (if (exist? "ttype") (string-upcase! (get "ttype"))
                 (string-append tst "_${f}") ))
     (set! next  (if (exist? "next") (string-upcase! (get "next")) tst))

     (shellf
       "for f in ${tk_list} ; do
       eval FSM_TRANS_%s_${f}=\"'{ FSS_%s, FSX_%s }'\"
       done"
       tst next ttype )        =][=

   ELSE                        =][=

     FOR t_st                  =][=

       (set! tst (string-upcase! (get "t_st")))
       (set! next  (if (exist? "next") (string-upcase! (get "next")) tst))
       =][=

       FOR ttkn                =][=

         ;;  This transition applies for all transitions in a certain state
         ;;
         (set! ttkn  (string-upcase! (get "ttkn")))
         (set! ttype (if (exist? "ttype") (string-upcase! (get "ttype"))
                     (string-append tst "_" ttkn) ))
    
         (shellf
           "for f in ${tk_list} ; do
           eval FSM_TRANS_%s_%s=\"'{ FSS_%s, FSX_%s }'\"
           done"
           tst ttkn next ttype )
         =][=

       ENDFOR  ttkn            =][=
     ENDFOR    t_st            =][=
   ENDIF t_st or ttkn is '*'   =][=
ENDFOR
=][=
`env | \
 egrep '^FSM_TRANS_' | \
 sed 's/.*, FSX_//;s/ .*/,/' | \
 sort -u > .fsm.xlist` =]

#ifdef DEFINE_FSM
/*[=#

   It is actually possible to have multiple specifications for a
   single state/token pair, but we unique sort the list and eliminate
   dups that way.  The unique-ified list is used to produce the switch
   statement later.

   The last state/token assignment will supply the value for the
   transition table.

=]
 *  This enumerates all the valid state + token-name transitions.
 */
typedef enum {
[=`sed 's/^/    FSX_/;$s/,$//' .fsm.xlist`=]
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
 */
static t_transition trans_table[ FSM_STATE_COUNT ][ [=(count "token")
                            =] ] = {[=

DEFINE state-table  =]
  { [=
  (set! tst (string-upcase! (get "state"))) =][=

  FOR token "\n    " =][=
    (shellf
      "echo \"$FSM_TRANS_%s_%s%s%s\""
      tst (string-upcase! (get "token"))
      (if (not (last-for?)) "," "")
      (if (first-for?) (sprintf "\t/* %s state */" (get "state")) "")
    )        =][=
  ENDFOR     =] }[=
ENDDEF       =][=

state-table state = init =][=

FOR state    =],
[=
state-table  =][=
ENDFOR       =]
};
#endif /* DEFINE_FSM */
#endif /* [=(. guard)=] */
[=
IF (exist? "example")  =][=

  (shellf
    "targ_file=%s-fsm.c
     [ -f ${targ_file} ] && mv -f ${targ_file} .fsm.oldtext"
    (base-name))

  (out-switch (string-append (base-name) "-fsm.c"))
  (set-writable) =][=
ELSE             =][=
  (out-switch (string-append (base-name) "-fsm.c")) =][=
ENDIF            =][=

(dne " *  " "/*  ")  =]
 *
[=(bsd "AutoFSM" "Bruce Korb" " *  ")=]
 */
#ifndef EXAMPLE_TEXT

#include <stdio.h>
#include <stdlib.h>
#define  DEFINE_FSM
#include "[=(. hdrname)=]"

#ifndef tSCC
#  define tSCC static const char
#endif

/*
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
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
[=(shellf
"columns --spread=1 -I8 -S, -f'zSt%%s' <<'_EOF_'
Init
%s
_EOF_"
(string-capitalize! (join "\n" (stack "state")))  )=] };
[=

FOR token =]
    tSCC zTk[=(string-capitalize! (get "token"))=][] = [=
       (c-string (string-downcase! (get "token")))=];[=
ENDFOR

=]
    tSCC* apzTokens[] = {
[=(shellf
"columns --spread=1 -I8 -S, -f'zTk%%s' <<'_EOF_'
%s
_EOF_"
(string-capitalize! (join "\n" (stack "token")))  )=] };

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

IF (exist? "example") =]

#else /* *is* [=example=]-type EXAMPLE_TEXT */[=

  CASE example  =][=

  =*  glob      =][=
	  (define example-type "global") =][=
      include "fsm-example.tpl"      =][=

  =*  fun       =][=
	  (define example-type "func")   =][=
      include "fsm-example.tpl"      =][=

  =*  state     =][=
	  (define example-type "state")  =][=
      include "fsm-state.tpl"        =][=

  *             =][=
      (shellf "rm -f .fsm.* >&2")
	  (error "Example must specify 'glob'al, 'state' or 'fun'ction")  =][=
  ESAC          =][=

ENDIF

=][=

`rm -f .fsm.* >&2`
=]
#endif /* EXAMPLE_TEXT */
