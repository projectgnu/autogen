< AutoGen5 template  -*- Mode: html -*-

# $Id: strsignal.tpl,v 3.2 2003/02/16 00:04:40 bkorb Exp $

(setenv "SHELL" "/bin/sh")

h >
/*
<(dne " *  ")>
 *
 *  copyright 1992-2003 Bruce Korb
 *
<(gpl "AutoGen" " *  ")>
 */
#ifndef MAX_SIGNAL_NUMBER
#define MAX_SIGNAL_NUMBER < (high-lim "signal") >
#ifndef tSCC
#  define tSCC static const char
#endif
<
FOR signal (for-from 0) (for-by 1) ><
  IF (exist? "signame") >
tSCC zSig_< % signame (sprintf "%%-13s" "%s[]")
                 > = "SIG<signame>";<
  (out-push-add ".sig.liststr") >
tSCC zInf_< % signame (sprintf "%%-13s" "%s[]")
                 > = < (c-string (get "sigtext")) >;<
  (out-pop) ><
  ELSE  >
tSCC zBad_< (sprintf "%-13s" (sprintf "%d[]" (for-index)))
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

tSCC* signal_names[] = {
<`columns -I4 -S, -i .sig.names` > };

#ifndef HAVE_SYS_SIGLIST
<`cat .sig.liststr`>

tSCC* sys_siglist[] = {
<`columns -I4 -S, -i .sig.list ; rm -f .sig.*` > };

#endif /* HAVE_SYS_SIGLIST */
#endif /* MAX_SIGNAL_NUMBER */
