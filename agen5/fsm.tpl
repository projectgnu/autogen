[= AutoGen5 Template  -*- Mode: text -*-

h=%s-fsm.h

c=%s-fsm.c

(setenv "SHELL" "/bin/sh")

=]
[=

CASE (suffix) =][=

== h =][=

  INCLUDE "fsm-trans.tpl" =][=
  INCLUDE "fsm-macro.tpl" =][=

  preamble     

=]
/*
 *  This file enumerates the states and transition tokens for a FSM.
 *
 *  te_[=(. pfx)=]_state
 *      The available states.  FSS_INIT is always defined to be zero
 *      and FSS_INVALID and FSS_DONE are always made the last entries.
 *
 *  te_[=(. pfx)=]_token
 *      The transition tokens.  These enumerate the token values used
 *      to select the next state from the current state.
 *      [=(. PFX)=]_EV_INVALID is always defined at the end.
 */
#ifndef [=(. guard)=]
#define [=(. guard)=]

/*
 *  Finite State machine States
 *
 *  Count of non-terminal states.  [=(. PFX)=]_ST_INVALID and [=(. PFX)=]_ST_DONE
 *  are terminal, [=(. PFX)=]_ST_INIT is not  :-).
 */
#define [=(. PFX)=]_STATE_CT  [=(+ 1 (count "state"))=]
typedef enum {
[=
  (shellf "${COLUMNS_EXE-columns} --spread=1 -I4 -S, -f'%s_ST_%%s' <<_EOF_
INIT
%s
INVALID
DONE
_EOF_" PFX (string-upcase! (join "\n" (stack "state"))) )=]
} te_[=(. pfx)=]_state;

/*
 *  Finite State machine transition Events.
 *
 *  Count of the valid transition events
 */
#define [=(. PFX)=]_EVENT_CT [=(count "event")=]
typedef enum {
[= compute-transitions =][=
  (shellf "${COLUMNS_EXE-columns} --spread=1 -I4 -S, -f'%s_EV_%%s' <<_EOF_
%s
INVALID
_EOF_" PFX (string-upcase! (join "\n" (stack "event"))) )=]
} te_[=(. pfx)=]_event;
[=

  CASE method  =][=

  =*  call     =][=

  =*  case     =][=

  ==  ""       =][=
    enumerate-transitions  use_ifdef = yes  =][=
  =*  no       =][=
    enumerate-transitions  use_ifdef = yes  =][=
  *            =][=
    (error (string-append "unknown/invalid FSM method:  " (get "method"))) =][=
  ESAC         =][=

  CASE type    =][=

  =* step      =]
/*
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  [=(. PFX)=]_ST_DONE or [=(. PFX)=]_ST_INVALID, it resets to
 *  [=(. PFX)=]_ST_INIT and returns [=(. PFX)=]_ST_INVALID.
 */
extern te_[=(. pfx)=]_state [=(. pfx)=]_step( te_[=(. pfx)
          =]_event[= % cookie[0] ", %s"=] );[=

  =* loop      =]
/*
 *  Call the FSM.  Will return [=(. PFX)=]_ST_DONE or [=(. PFX)=]_ST_INVALID
 */
extern te_[=(. pfx)=]_state [=(. pfx)=]_run_fsm( [=
  IF (exist? "cookie") =][=
    FOR cookie ", " =][=cookie=][=ENDFOR=][=
  ELSE=]void[=ENDIF=] );[=

  == ""        =][=

  *            =][=
    (error (string-append "unknown/invalid FSM type:  " (get "type"))) =][=
  ESAC         =]
#endif /* [=(. guard)=] */[=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#   C OUTPUT BEGINS HERE
#
=][=

== c =][=

  (if (~ (get "method") "|no.*") (out-delete)) =][=

  preamble     

=]
#define DEFINE_FSM
#include "[=(. hdrname)=]"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
[=(extract fsm-source "/* %s === USER HEADERS === %s */")=]

#ifndef NULL
#  define NULL 0
#endif

#ifndef tSCC
#  define tSCC static const char
#endif
[= CASE method  =][=

  =* "case"       =][=
   enumerate-transitions =][=
  =* "call"       =][=
   callback-transitions  =][=
  ESAC            =]
[=IF (=* (get "type") "step")=]
/*
 *  The FSM machine state
 */
static te_[=(. pfx)=]_state [=(. pfx)=]_state = [=(. PFX)=]_ST_INIT;
[=ENDIF=]
[= emit-invalid-msg =][=

  IF  (=* (get "method") "call")        =][=

    `set -- \`sed 's/,//' .fsm.xlist\`` =][=

    WHILE `echo $#`     =][=

      invoke build-callback
        cb_prefix = (string-append pfx "_do")
        cb_name   = (shell "echo $1 ; shift") =][=

    ENDWHILE  echo $#   =][=

  ENDIF                 =][=


  CASE type             =][=

  =*  loop              =][=
    looping-machine     =][=
  =*  step              =][=
    stepping-machine    =][=
  ESAC                  =][=

ESAC (suffix)           =][=

trailer =][=


DEFINE emit-invalid-msg =]
/*
 *  Define all the event and state names
 */
tSCC zBogus[]     = "** OUT-OF-RANGE **";
tSCC zStInit[]    = "init";
tSCC zEvInvalid[] = "* Invalid Event *";
tSCC zFsmErr[]    =
    "FSM Error:  in state %d (%s), event %d (%s) is invalid\n";
[=
  FOR state
=]
tSCC zSt[=(string-capitalize! (get "state"))=][] = [=
        (c-string (string-downcase! (get "state")))=];[=
  ENDFOR

=]
tSCC* apzStates[] = {
[=(shellf
"${COLUMNS_EXE-columns} --spread=1 -I4 -S, -f'zSt%%s' <<'_EOF_'
Init
%s
_EOF_"
  (string-capitalize! (join "\n" (stack "state")))  )=] };
[=

  FOR event =]
tSCC zEv[=(string-capitalize! (get "event"))=][] = [=
       (c-string (string-downcase! (get "event")))=];[=
  ENDFOR

=]
tSCC* apzEvents[] = {
[=(shellf
"${COLUMNS_EXE-columns} --spread=1 -I4 -S, -f'zEv%%s' <<'_EOF_'
%s
Invalid
_EOF_"
  (string-capitalize! (join "\n" (stack "event")))  )=] };

#define [=(. PFX)=]_EVT_NAME(t) ( (((unsigned)(t)) >= [=(. PFX)=]_EV_INVALID) \
    ? zBogus : apzEvents[ t ])

#define [=(. PFX)=]_STATE_NAME(s) ( (((unsigned)(s)) > [=(. PFX)=]_ST_INVALID) \
    ? zBogus : apzStates[ s ])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
int
[=(. pfx)=]_invalid_transition( te_[=(. pfx)=]_state st, te_[=
  (. pfx)=]_event evt )
{
[=(extract fsm-source "    /* %s == INVALID TRANS MSG == %s */" ""
  (sprintf
"    fprintf( stderr, zFsmErr, st, %s_STATE_NAME( st ), evt, %s_EVT_NAME( evt ));" PFX PFX) )=]

    return EXIT_FAILURE;
}
[=



ENDDEF                  =]

