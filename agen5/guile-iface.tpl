[= AutoGen5 Template h =]
[=

(out-push-new)  =]
set -- $(sort -n -u <<_EOF_
[=  (join "\n" (stack "iface.i-impl.i-end")) =]
_EOF_
)
v_list="$*"
r_list=$(ix=$# ; while (( ix > 0 )) ; do eval echo \${$ix}
         (( ix = ix - 1 )) ; done)
i_list="[= (join " " (stack "iface.i-name")) =]"
PS4='>${FUNC_NAME:-ag}> '

fill_in() {
  for v in $r_list
  do
    eval f=\"\${${name}_vals[$v]}\"
    if test ${#f} -gt 0
    then val="$f"
    else eval ${name}_vals[$v]=\"$val\"
    fi
  done
}

prt_tbl() {
  if='#if  '
  for v in $v_list
  do
    printf '%s GUILE_VERSION < %s000\n' "$if" "$v"
    if='#elif'

    for i in $i_list
    do
      NM=$(echo $i | tr a-z- A-Z_)
      eval NM=\"$NM\(\${${i}_args}\)\"
      eval code=\"\${${i}_vals[$v]}\"
      if test ${#code} -lt 40 -a ${#NM} -lt 22
      then
        printf '# define AG_SCM_%-21s %s\n' "${NM}" "${code}"
      else
        printf '# define AG_SCM_%-21s \\\n    %s\n' "${NM}" "${code}"
      fi
    done

    echo
  done
}
[=
(shell (out-pop #t)) =][=

FOR iface       =][=

  (shell (string-append
    "val='" (get "i-impl[].i-code") "'\n"
    (get "i-name") "_args='" (get "i-args") "'\n"
    "name=" (get "i-name")
  ))            =][=

  FOR i-impl    =][=

    (shell (string-append
      (get "i-name") "_vals[" (get "i-end")  "]='" (get "i-code") "'"
    ))          =][=

  ENDFOR        =][=

  `fill_in`     =][=

ENDFOR iface    =][=

`prt_tbl`

=]

# define scm_sizet                    size_t

#else
#error unknown GUILE_VERSION
#endif

static inline SCM ag_eval(char const * pzStr)
{
    SCM res;
    char const * pzSaveScheme = pzLastScheme; /* Watch for nested calls */
    pzLastScheme = pzStr;

    res = ag_scm_c_eval_string_from_file_line(
        pzStr, pCurTemplate->pzTplFile, pCurMacro->lineNo);

    pzLastScheme = pzSaveScheme;
    return res;
}
