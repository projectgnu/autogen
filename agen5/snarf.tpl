[= autogen template ini =]
/*
[=_EVAL " *  " _DNE=]
 *
 *  Guile Initializations - [=group " " + _CAP =]Global Variables
 */
typedef SCM (t_scm_callout_proc)();[=

  _FOR gfunc =]
static const char s_[=name #[] + %-26s _printf =] = [=
    _IF string _exist =][=string _str=][=
    _ELSE =][=
      name _str "echo %s |
                 sed -e's/_p$/?/' -e's/_x$/!/' -e's/_/-/g' -e's/_to_/->/'"
           _printf _shell _str =][=
    _ENDIF =];[=
  /gfunc =][=

  _FOR gfunc =]
extern t_scm_callout_proc [=group #_ + =]scm_[=name=];[=
  /gfunc =][=

  _FOR syntax =]
static const char s_[=name #[] + %-26s _printf =] = [=
    _IF string _exist =][=string _str=][=
    _ELSE =][=
      name _str "echo %s |
                 sed -e's/_p$/?/' -e's/_x$/!/' -e's/_/-/g' -e's/_to_/->/'"
           _printf _shell _str =][=
    _ENDIF =];[=
  /syntax =][=

  _FOR symbol =]
[=  _IF global _exist=]      [=_ELSE=]static[=_ENDIF
    =] SCM sym_[=name %-18s _printf =] = SCM_BOOL_F;[=
  /symbol =]

/*
 *  [=group " " + _CAP =]Initialization procedure
 */
void
[=group #_ + =]init( void )
{[=
  _IF init_code _exist=]
  [=init_code=][=
  _ENDIF =][=

  _FOR gfunc =]
  gh_new_procedure( (char*)s_[=name=],
                      [=group #_ + =]scm_[=name #, + %-28s _printf=] [=
      _IF req _exist =][=req=][=_ELSE=]1[=_ENDIF=], [=
      _IF opt _exist =][=opt=][=_ELSE=]0[=_ENDIF=], [=
      _IF var _exist =][=var=][=_ELSE=]0[=_ENDIF=] );[=
  /gfunc =][=

  _FOR syntax =]
  scm_make_synt( s_[=name #, + %-16s _printf =] [=type=], [=cfn=] );[=
  /syntax =][=

  _FOR symbol =][=
    _IF vcell _exist ! =]
  sym_[=name=] = scm_permanent_object (SCM_CAR (scm_intern0 ([=
                 scheme_name=])));[=
    _ELSE =]
  sym_[=name=] = scm_permanent_object (scm_intern0 ([=scheme_name=]));
  SCM_SETCDR (sym_[=name=], [=
      _IF    init_val _exist =][=init_val=][=
      _ELIF const_val _exist =]scm_long2num([=const_val=])[=
      _ELSE                  =]SCM_BOOL_F[=_ENDIF=]);[=
    _ENDIF =][=
  /symbol =][=


  _IF fini_code _exist=]
  [=fini_code=][=
  _ENDIF =]
}[=

end of snarf.tpl =]
