{#autogen template 
# $Id: strsignal.tpl,v 2.0 1998/09/22 22:17:26 bkorb Exp $
h #}
/*
{#_eval "# *  " _DNE#}
 *
{#_eval strsignal "Bruce Korb" "# *  " _LGPL#}
 */
#ifndef MAX_SIGNAL_NUMBER
#define MAX_SIGNAL_NUMBER {#_eval signal _hilim#}
{#
_FOR signal FROM 0 BY 1#}{#
  _IF signame _exist#}
static char zSig_{#signame _up #[] + "#%-13s" _printf#} = "SIG{#signame#}";
static char zInf_{#signame _up #[] + "#%-13s" _printf#} = "{#sigtext#}";{#
  _ELSE#}
static char zBad_{#_eval _index "#%d" _printf "#[]" +
                    "#%-13s" _printf#} = "Signal {#_eval _index#} invalid";{#
  _ENDIF#}{#
/signal#}

static char* sys_siglist[] = {{#_FOR signal FROM 0 BY 1
     #}{#
      _IF _index 3 % 0 =#}
{#    _ENDIF #}  {#
      _eval _index "#/* %2d */ " _printf
      #}{#

      _IF _last
        #}zInf_{#signame _up#}{#
      _ELIF signame _exist #}{#
        signame _up #, + "zInf_%-9s" _printf #}{#
      _ELSE #}{#
        _eval _index #%d, _printf # + "zBad_%-9s" _printf #}{#
      _ENDIF #}{#

/signal #} };

static char* signal_names[] = {{#_FOR signal FROM 0 BY 1
    #}{#
      _IF _index 5 % 0 =#}
    {#_ENDIF#}{#
      _IF _last
        #}zSig_{#signame _up#}{#
      _ELIF signame _exist
        #}{#signame _up #, + "zSig_%-9s" _printf #}{#
      _ELSE
        #}{#_eval _index #%d, _printf # + "zBad_%-9s" _printf #}{#
      _ENDIF #}{#
/signal #} };

#endif /* MAX_SIGNAL_NUMBER */
