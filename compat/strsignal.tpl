< AutoGen5 template  -*- Mode: html -*-

# $Id: strsignal.tpl,v 3.0 2001/12/09 19:46:02 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h >
/*
<(dne " *  ")>
 *
 *  copyright 1992-1999 Bruce Korb
 *
<(gpl "AutoGen" " *  ")>
 */
#ifndef MAX_SIGNAL_NUMBER
#define MAX_SIGNAL_NUMBER < (high-lim "signal") >
<
FOR signal (for-from 0) (for-by 1) ><
  IF (exist? "signame") >
static char zSig_< % signame (sprintf "%%-13s" "%s[]")
                 > = "SIG<signame>";<
  (out-push-add ".sig.liststr") >
static char zInf_< % signame (sprintf "%%-13s" "%s[]")
                 > = < (c-string (get "sigtext")) >;<
  (out-pop) ><
  ELSE  >
static char zBad_< (sprintf "%-13s" (sprintf "%d[]" (for-index)))
                  > = "Signal <(for-index)> invalid";<
  ENDIF ><
  ;; evaluations
  (out-push-add ".sig.list")
  (sprintf "/* %2d */ %s\n" (for-index) (if (exist? "signame")
           (string-append "zInf_" (get "signame"))
           (sprintf "zBad_%d" (for-index)) )) ><
  (out-pop)

  (out-push-add ".sig.names")
  (if (exist? "signame")
      (string-append "zSig_" (get "signame") "\n")
      (sprintf "zBad_%d\n" (for-index)) ) ><
  (out-pop) ><

ENDFOR signal>

static char* signal_names[] = {
<`columns -I4 -S, -i .sig.names` > };

#ifndef HAVE_SYS_SIGLIST
<`cat .sig.liststr`>

static char* sys_siglist[] = {
<`columns -I4 -S, -i .sig.list ; rm -f .sig.*` > };

#endif /* HAVE_SYS_SIGLIST */
#endif /* MAX_SIGNAL_NUMBER */
