[= AutoGen5 Template  -*- Mode: text -*-

h=%s-fsm.h

c=%s-fsm.c

(setenv "SHELL" "/bin/sh")
(define fmt "")
(shellf "[ -f %1$s-fsm.h ] && mv -f %1$s-fsm.h .fsm.head
[ -f %1$s-fsm.c ] && mv -f %1$s-fsm.c .fsm.code" (base-name))

#  AutoGen copyright 1992-2006 Bruce Korb

=]
[=

CASE (suffix) =][=#

PURPOSE:
   This template collection will produce a finite state machine based on a
   description of such a machine.  The presumption is that you generally
   do not need to know what the current state is in order to determine what
   kind of transition is to be taken.  That is to say, there is a global
   transition table that is indexed by the current state and the next
   transition type to determine the next state and trigger any optional
   transition handling code.

   The finite state machine may be either the master, driving the other
   parts of the program, or act as a subroutine keeping track of state
   between calls.  Consequently, the "type" attribute may be set to:

   looping
       If the machine processes transitions until it reaches a terminal
       state (error or done).
   stepping
       If the FSM code will process a single transition and then return.
   reentrant
       This method is much the same as stepping, except that the caller
       must save the current state and provide it on the next call.
       In this fashion, an FSM may be used in a multi threaded application.

   The machine can be constructed in either of three formats, depending
   on the value of the "method" attribute:

   callout
       This method will use a callout table instead of a switch statement
       to implement the machine.
   case
       This is the alternate implementation method.
   none
       Do not supply the "method" attribute.  Choosing this will cause only
       the dispatch table to be created.  The implementation code is omitted.
       The "type" attribute is then ignored.

YOU SUPPLY:
   "state"      The list of valid state names.  The "init" and "done" states
                are automatically added to this.  If there are other terminal
                states, they must set "nxtSt" to "done".
   "event"      The list of valid transition types.
   "type"       The machine structure type:  looping, stepping or reentrant
   "method"     The implementation type: callout table, case statement or none.
   "prefix"     A prefix to glue onto the front of created names
   "cookie"     zero, one or more of these each containing a C type and name
                suitable for use in a procedure header.  It is used to pass
                through arguments to implementation code.

   "transition" Define the handling for a transition from one state to another.
                It contains:
       "tst"    the starting state(s).  This may be one, or a list or '*'
                to indicate all states.
       "tev"    the event that triggers this transition.  This may also be
                a list of events or a '*'.
       "ttype"  the transition type.  By default it is named after the state
                and event names, but by specifying a particular type, multiple
                different transitions may invoke the same handling code.
       "next"   the presumptive destination state.  "presumptive" because
                the code that handles the transition may select a different
                destination.  Doing that will violate mathematical models, but
                it often makes writing this stuff easier.

THE TEMPLATE PRODUCES:

   a header file enumerating the states and events, and declaring the FSM
   state machine procedure.

   the code file with the implementation (provided "method" was specified).
   This source file contains special comments around code that is to be
   preserved from one regeneration to the next.  BE VERY CAREFUL: if you
   change the name of a state or event, any code that was stored under the
   old name will not be carried forward and is likely to get lost.
   If you change names, then *save your work*.

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

=][=

== h =][=

  INCLUDE "fsm-trans.tpl" =][=
  INCLUDE "fsm-macro.tpl" =][=

  INVOKE  preamble

=]
/*
 *  This file enumerates the states and transition events for a FSM.
 *
 *  te_[=(. pfx)=]_state
 *      The available states.  FSS_INIT is always defined to be zero
 *      and FSS_INVALID and FSS_DONE are always made the last entries.
 *
 *  te_[=(. pfx)=]_event
 *      The transition events.  These enumerate the event values used
 *      to select the next state from the current state.
 *      [=(. PFX)=]_EV_INVALID is always defined at the end.
 */
[=(make-header-guard "autofsm")=]

/*
 *  Finite State machine States
 *
 *  Count of non-terminal states.  The generated states INVALID and DONE
 *  are terminal, but INIT is not  :-).
 */
#define [=(. PFX)=]_STATE_CT  [=(+ 1 (count "state"))=]
typedef enum {
[=
  (shellf "${CLexe-columns} --spread=1 -I4 -S, -f'%s_ST_%%s' <<_EOF_
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
  (shellf "${CLexe-columns} --spread=1 -I4 -S, -f'%s_EV_%%s' <<_EOF_
%s
INVALID
_EOF_" PFX (string-upcase! (join "\n" (stack "event"))) )=]
} te_[=(. pfx)=]_event;
[=

  CASE method     =][=

  ~*  call|case   =][=

    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
    #
    #   We are implementing the machine.  Declare the external  =][=

    CASE type     =][=

    ~* step|reent =][= make-step-proc mode = "extern " =];[=

    =* loop       =][= make-loop-proc mode = "extern " =];[=

    *             =][=
    (error (string-append "invalid FSM type:  ``" (get "type")
           "'' must be ``looping'', ``stepping'' or ``reentrant''" ))
    =][=
    ESAC          =][=

    #  End external procedure declarations
    #
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
  #
  #   We are *NOT* implementing the machine.  Define the table  =][=

  ==  ""       =][=
    enumerate-transitions  use_ifdef = yes  =][=
  =*  no       =][=
    enumerate-transitions  use_ifdef = yes  =][=
  *            =][=
    (error (sprintf
        "invalid FSM method:  ``%s'' must be ``callout'', ``case'' or ``none''"
        (get "method"))) =][=
  ESAC         =]

#endif /* [=(. header-guard)=] */[=

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#   C OUTPUT BEGINS HERE
#
=][=

== c =][=

  (if (~ (get "method") "(no.*){0,1}")
      (begin (shell "rm -f .fsm.*") (out-delete))  ) =][=

  INVOKE preamble

=]
#define DEFINE_FSM
#include "[=(. header-file)=]"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
[=(extract fsm-source "/* %s === USER HEADERS === %s */")=]

#ifndef NULL
#  define NULL 0
#endif
[= CASE method          =][=
   =* "case"            =][= enumerate-transitions =][=
   =* "call"            =][= callback-transitions  =][=
   ESAC                 =]
[=IF (=* (get "type") "step")=]
/*
 *  The FSM machine state
 */
static te_[=(. pfx)=]_state [=(. pfx)=]_state = [=(. PFX)=]_ST_INIT;
[=ENDIF=]
[= emit-invalid-msg     =][=

  IF  (=* (get "method") "call")        =][=

    `set -- \`sed 's/,//' .fsm.xlist\`` =][=

    WHILE `echo $#`     =][=

      INVOKE build-callback
        cb_prefix = (string-append pfx "_do")
        cb_name   = (shell "echo $1 ; shift") =][=

    ENDWHILE  echo $#   =][=
  ENDIF                 =][=

  CASE type             =][=
  =*   loop             =][= looping-machine  =][=
  ~*   step|reent       =][= stepping-machine =][=
  ESAC                  =][=

  `rm -f .fsm.*`        =][=

ESAC (suffix)

=]
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of [=(out-name)=] */
