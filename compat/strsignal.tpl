(-: AutoGen5 template 

# $Id: strsignal.tpl,v 2.2 1999/10/28 02:23:36 bruce Exp $

h :-)
/*
[=(dne " *  ")=]
 *
 *  copyright 1992-1999 Bruce Korb
 *
[=(gpl "AutoGen" " *  ")=]
 */
#ifndef MAX_SIGNAL_NUMBER
#define MAX_SIGNAL_NUMBER (-: (high-lim "signal") :-)
(-:
FOR signal (for-from 0) (for-by 1) :-)(-:
  IF (exist? "signame") :-)
static char zSig_(-: % signame (sprintf "%%-13s" "%s[]"):-) =
"SIG(-:signame:-)";(-:
  (out-push-add .sig.liststr) :-)
static char zInf_(-: % signame (sprintf "%%-13s" "%s[]"):-) = "(-:sigtext:-)";(-:
  (out-pop) :-)(-:
  ELSE  :-)
static char zBad_(-: (sprintf "%-13s" (sprintf "%d[]" (for-index)))
                  :-) = "Signal (-:(for-index):-) invalid";(-:
  ENDIF :-)(-:
  ;; evaluations
  (out-push-add ".sig.list")
  (sprintf "/* %2d */ %s\n" (for-index) (if (exist? "signame")
           (string-append "zInf_" (get "signame"))
           (sprintf "zBad_%d" (for-index)) )) :-)(-:
  (out-pop)

  (out-push-add ".sig.names")
  (if (exist? "signame")
      (string-append "zSig_" (get "signame") "\n")
      (sprintf "zBad_%d\n" (for-index)) ) :-)(-:
  (out-pop) :-)(-:

ENDFOR signal:-)
(-:`cat .sig.liststr ; rm -f .sig.liststr`:-)
#ifndef HAVE_SYS_SIGLIST

static char* sys_siglist[] = {
(-:`columns -I4 -S, -i .sig.list ; rm -f .sig.list` :-) };
#endif /* HAVE_SYS_SIGLIST */

static char* signal_names[] = {
(-:`columns -I4 -S, -i .sig.names ; rm -f .sig.names` :-) };

#endif /* MAX_SIGNAL_NUMBER */
