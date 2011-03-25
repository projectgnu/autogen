/**
 *  \file agUtils.c
 *
 * Various utilities for AutoGen.
 *
 *  Time-stamp:        "2011-03-25 14:18:41 bkorb"
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* = = = START-STATIC-FORWARD = = = */
static void
define_base_name(void);

static void
put_defines_into_env(void);

static void
start_dep_file(void);

static char const *
skipQuote(char const * pzQte);
/* = = = END-STATIC-FORWARD = = = */

#ifndef HAVE_STRLCPY
size_t
strlcpy(char* dest, char const * src, size_t n)
{
    char* pz = dest;
    char const * ps = src;
    size_t sz = 0;

    for (;;) {
        if ((*(pz++) = *(src++)) == NUL)
            break;

        /*
         *  This is the unusual condition.  Do the exceptional work here.
         */
        if (--n <= 0) {
            pz[-1] = NUL;
            sz = strlen(src) + 1; /* count of chars not copied out */
            break;
        }
    }

    return sz + (src - ps);
}
#endif


LOCAL char *
aprf(char const * pzFmt, ...)
{
    char* pz;
    va_list ap;
    va_start(ap, pzFmt);
    (void)vasprintf(&pz, pzFmt, ap);
    va_end(ap);

    if (pz == NULL) {
        static char const zMsg[] =
            "could not allocate for or formatting failed on:\n%s";
        char z[ sizeof (zMsg) + SCRIBBLE_SIZE ];
        snprintf(z, sizeof(z), zMsg, pzFmt);
        AG_ABEND(z);
    }
    return pz;
}

static void
define_base_name(void)
{
    char const *  pz;
    char* pzD;

    if (! ENABLED_OPT(DEFINITIONS)) {
        OPT_ARG(BASE_NAME) = "baseless";
        return;
    }

    pz = strrchr(OPT_ARG(DEFINITIONS), '/');
    /*
     *  Point to the character after the last '/', or to the full
     *  definition file name, if there is no '/'.
     */
    pz = (pz == NULL) ? OPT_ARG(DEFINITIONS) : (pz + 1);

    /*
     *  IF input is from stdin, then use "stdin"
     */
    if ((pz[0] == '-') && (pz[1] == NUL)) {
        OPT_ARG(BASE_NAME) = "stdin";
        return;
    }

    /*
     *  Otherwise, use the basename of the definitions file
     */
    OPT_ARG(BASE_NAME) = \
        pzD = AGALOC(strlen(pz)+1, "derived base name");

    while ((*pz != NUL) && (*pz != '.'))  *(pzD++) = *(pz++);
    *pzD = NUL;
}

static void
put_defines_into_env(void)
{
    int     ct  = STACKCT_OPT(DEFINE);
    char const **   ppz = STACKLST_OPT(DEFINE);

    do  {
        char const * pz = *(ppz++);
        /*
         *  IF there is no associated value,  THEN set it to '1'.
         *  There are weird problems with empty defines.
         *  FIXME:  we loose track of this memory.  Don't know what to do,
         *  really, there is no good recovery mechanism for environment
         *  data.
         */
        if (strchr(pz, '=') == NULL) {
            size_t siz = strlen(pz)+3;
            char*  p   = AGALOC(siz, "env define");

            strcpy(p, pz);
            strcpy(p+siz-3, "=1");
            pz = p;
        }

        /*
         *  Now put it in the environment
         */
        putenv((char*)pz);
    } while (--ct > 0);
}

static void
start_dep_file(void)
{
    static char const mkdep_fmt[] =
        "# Makefile dependency file created by %s\n"
        "# with the following command:\n";

    char * pz;

    if (pzDepFile == NULL)
        pzDepFile = aprf("%s.d-XXXXXX", OPT_ARG(BASE_NAME));
    mkstemp((char *)pzDepFile);
    pfDepends = fopen(pzDepFile, "w");

    if (pfDepends == NULL)
        AG_CANT("fopen for write", pzDepFile);

    fprintf(pfDepends, mkdep_fmt, autogenOptions.pzProgName);

    {
        int     ac = autogenOptions.origArgCt;
        char ** av = autogenOptions.origArgVect;

        while (ac-- > 0) {
            char * arg = *(av++);
            fprintf(pfDepends, "#   %s\n", arg);
        }
    }

    if (pzDepTarget == NULL) {
        char * p;
        AGDUPSTR(pzDepTarget, pzDepFile, "targ name");
        p  = (char *)pzDepTarget + strlen(pzDepTarget) - 7;
        *p = NUL;
    }

    {
        static char const tfmt[] = "%s_%s";
        char const * pnm = autogenOptions.pzPROGNAME;
        char const * bnm = strchr(pzDepTarget, '/');

        if (bnm != NULL)
            bnm++;
        else
            bnm = pzDepTarget;

        {
            size_t sz = strlen(pnm) + strlen(bnm) + sizeof(tfmt) - 4;

            pz_targ_base = pz = AGALOC(sz, "mk targ list");
            sprintf(pz, tfmt, pnm, bnm);
        }
    }

    /*
     * Now scan over the characters in "pz_targ_base".  Anything that
     * is not a legal name character gets replaced with an underscore.
     */
    for (;;) {
        unsigned int ch = (unsigned int)*(pz++);
        if (ch == NUL)
            break;
        if (! isalnum(ch))
            pz[-1] = '_';
    }
    fprintf(pfDepends, "%s_TList =", pz_targ_base);
}

LOCAL void
doOptions(int arg_ct, char ** arg_vec)
{
    /*
     *  Advance the argument counters and pointers past any
     *  command line options
     */
    {
        static char const one_src[] = "%s ERROR:  Too many definition files\n";
        int  optCt = optionProcess(&autogenOptions, arg_ct, arg_vec);

        /*
         *  Make sure we have a source file, even if it is "-" (stdin)
         */
        switch (arg_ct - optCt) {
        case 1:
            if (! HAVE_OPT(DEFINITIONS)) {
                OPT_ARG(DEFINITIONS) = *(arg_vec + optCt);
                break;
            }
            /* FALLTHROUGH */

        default:
            usage_message(one_src, pzProg);
            /* NOTREACHED */

        case 0:
            if (! HAVE_OPT(DEFINITIONS))
                OPT_ARG(DEFINITIONS) = "-";
            break;
        }
    }

    startTime = time(NULL);

    if (! HAVE_OPT(TIMEOUT))
        OPT_ARG(TIMEOUT) = (char const *)AG_DEFAULT_TIMEOUT;

    /*
     *  IF the definitions file has been disabled,
     *  THEN a template *must* have been specified.
     */
    if (  (! ENABLED_OPT(DEFINITIONS))
       && (! HAVE_OPT(OVERRIDE_TPL)) )
        AG_ABEND("no template was specified");

    /*
     *  IF we do not have a base-name option, then we compute some value
     */
    if (! HAVE_OPT(BASE_NAME))
        define_base_name();

    if (HAVE_OPT(MAKE_DEP))
        start_dep_file();

    strequate(OPT_ARG(EQUATE));

    /*
     *  IF we have some defines to put in our environment, ...
     */
    if (HAVE_OPT(DEFINE))
        put_defines_into_env();
}

LOCAL char const *
getDefine(char const * pzDefName, ag_bool check_env)
{
    char const **   ppz;
    int     ct;
    if (HAVE_OPT(DEFINE)) {
        ct  = STACKCT_OPT( DEFINE);
        ppz = STACKLST_OPT(DEFINE);

        while (ct-- > 0) {
            char const * pz   = *(ppz++);
            char * pzEq = strchr(pz, '=');
            int    res;

            if (pzEq != NULL)
                *pzEq = NUL;

            res = strcmp(pzDefName, pz);
            if (pzEq != NULL)
                *pzEq = '=';

            if (res == 0)
                return (pzEq != NULL) ? pzEq+1 : zNil;
        }
    }
    return check_env ? getenv(pzDefName) : NULL;
}


/*
 *  The following routine scans over quoted text, shifting
 *  it in the process and eliminating the starting quote,
 *  ending quote and any embedded backslashes.  They may
 *  be used to embed the quote character in the quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
LOCAL char*
spanQuote(char* pzQte)
{
    char  q = *pzQte;          /*  Save the quote character type */
    char* p = pzQte++;         /*  Destination pointer           */

    while (*pzQte != q) {
        switch (*p++ = *pzQte++) {
        case NUL:
            return pzQte-1;      /* Return address of terminating NUL */

        case '\\':
            if (q != '\'') {
                int ct = ao_string_cook_escape_char(pzQte, p-1, 0x7F);
                if (p[-1] == 0x7F)  p--;
                pzQte += ct;

            } else {
                switch (*pzQte) {
                case '\\':
                case '\'':
                case '#':
                    p[-1] = *pzQte++;
                }
            }
            break;

        default:
            ;
        }
    }

    *p = NUL;
    return pzQte+1; /* Return addr of char after the terminating quote */
}

/*
 *  The following routine skips over quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
static char const *
skipQuote(char const * pzQte)
{
    char  q = *pzQte++;        /*  Save the quote character type */

    while (*pzQte != q) {
        switch (*pzQte++) {
        case NUL:
            return pzQte-1;      /* Return address of terminating NUL */

        case '\\':
            if (q == '\'') {
                /*
                 *  Single quoted strings process the backquote specially
                 *  only in fron of these three characters:
                 */
                switch (*pzQte) {
                case '\\':
                case '\'':
                case '#':
                    pzQte++;
                }

            } else {
                char p[10];  /* provide a scratch pad for escape processing */
                int ct = ao_string_cook_escape_char(pzQte, p, 0x7F);
                pzQte += ct;
            } /* if (q == '\'')      */
        }     /* switch (*pzQte++)   */
    }         /* while (*pzQte != q) */

    return pzQte+1; /* Return addr of char after the terminating quote */
}


LOCAL char const *
skipScheme(char const * pzSrc,  char const * pzEnd)
{
    int  level = 0;

    for (;;) {
        if (pzSrc >= pzEnd)
            return pzEnd;
        switch (*(pzSrc++)) {
        case '(':
            level++;
            break;

        case ')':
            if (--level == 0)
                return pzSrc;
            break;

        case '"':
            pzSrc = skipQuote(pzSrc-1);
        }
    }
}


LOCAL int
count_nl(char const * pz)
{
    int ct = 0;
    for (;;) {
        char const * p = strchr(pz, '\n');
        if (p == NULL)
            break;
        ct++;
        pz = p + 1;
    }
    return ct;
}


LOCAL char const *
skipExpression(char const * pzSrc, size_t len)
{
    char const * pzEnd = pzSrc + len;

 guess_again:

    while (IS_WHITESPACE_CHAR(*pzSrc)) pzSrc++;
    if (pzSrc >= pzEnd)
        return pzEnd;
    switch (*pzSrc) {
    case ';':
        pzSrc = strchr(pzSrc, '\n');
        if (pzSrc == NULL)
            return pzEnd;
        goto guess_again;

    case '(':
        return skipScheme(pzSrc, pzEnd);

    case '"':
    case '\'':
    case '`':
        pzSrc = skipQuote(pzSrc);
        return (pzSrc > pzEnd) ? pzEnd : pzSrc;

    default:
        break;
    }

    while (! IS_WHITESPACE_CHAR(*pzSrc))  pzSrc++;
    return (pzSrc > pzEnd) ? pzEnd : pzSrc;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agUtils.c */
