
/**
 * @file functions.c
 *
 *  Time-stamp:        "2012-03-31 13:55:12 bkorb"
 *
 *  This module implements text functions.
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2012 by Bruce Korb - all rights reserved
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

/*=macfunc INCLUDE
 *
 *  what:   Read in and emit a template block
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *
 *  The entire contents of the named file is inserted at this point.
 *  The contents of the file are processed for macro expansion.  The
 *  arguments are eval-ed, so you may compute the name of the file to
 *  be included.  The included file must not contain any incomplete
 *  function blocks.  Function blocks are template text beginning with
 *  any of the macro functions @samp{CASE}, @samp{DEFINE}, @samp{FOR},
 *  @samp{IF} and @samp{WHILE}; extending through their respective
 *  terminating macro functions.
=*/
macro_t *
mFunc_Include(templ_t * tpl, macro_t * mac)
{
    bool         allocated_name;
    char const * fname = eval_mac_expr(&allocated_name);

    if (*fname != NUL) {
        templ_t * new_tpl = tpl_load(fname, tpl->td_file);
        macro_t * last_mac = new_tpl->td_macros + (new_tpl->td_mac_ct - 1);

        if (last_mac->md_code == FTYP_TEXT) {
            /*
             *  Strip off trailing white space from included templates
             */
            char * pz = new_tpl->td_text + last_mac->md_txt_off;
            pz = SPN_WHITESPACE_BACK(pz, pz);

            /*
             *  IF there is no text left, remove the macro entirely
             */
            if (pz != new_tpl->td_text + last_mac->md_txt_off) {
                *pz = NUL;
            } else {
                new_tpl->td_mac_ct--;
                last_mac--;
            }
        }

        if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE) {
            fprintf(trace_fp, TRACE_FN_INC_TPL, new_tpl->td_file);
            if (OPT_VALUE_TRACE == TRACE_EVERYTHING)
                fprintf(trace_fp, TRACE_FN_INC_LINE, current_tpl->td_file,
                        mac->md_line);
        }

        gen_block(new_tpl, new_tpl->td_macros, last_mac + 1);
        tpl_unload(new_tpl);
        current_tpl = tpl;
    }

    if (allocated_name)
        AGFREE((void*)fname);

    return mac + 1;
}


/**
 *  digest an INCLUDE macro.
 *
 *  Simply verify that there is some argument to this macro.
 *  Regular "expr" macros are their own argument, so there is always one.
 */
macro_t *
mLoad_Include(templ_t * tpl, macro_t * mac, char const ** ppz)
{
    if ((int)mac->md_res == 0)
        AG_ABEND_IN(tpl, mac, LD_INC_NO_FNAME);
    return mLoad_Expr(tpl, mac, ppz);
}


/*=macfunc UNKNOWN
 *
 *  what:  Either a user macro or a value name.
 *  handler_proc:
 *  load_proc:
 *  unnamed:
 *
 *  desc:
 *
 *  The macro text has started with a name not known to AutoGen.  If, at run
 *  time, it turns out to be the name of a defined macro, then that macro is
 *  invoked.  If it is not, then it is a conditional expression that is
 *  evaluated only if the name is defined at the time the macro is invoked.
 *
 *  You may not specify @code{UNKNOWN} explicitly.
=*/
macro_t *
mFunc_Unknown(templ_t * pT, macro_t * pMac)
{
    templ_t * pInv = find_tpl(pT->td_text + pMac->md_name_off);
    if (pInv != NULL) {
        if (OPT_VALUE_TRACE >= TRACE_EVERYTHING)
            fprintf(trace_fp, TRACE_FN_REMAPPED, TRACE_FN_REMAP_INVOKE,
                    pMac->md_code, pT->td_file, pMac->md_line);
        pMac->md_code    = FTYP_DEFINE;
        pMac->md_pvt = (void*)pInv;
        parse_mac_args(pT, pMac);
        return mFunc_Define(pT, pMac);
    }

    if (OPT_VALUE_TRACE >= TRACE_EVERYTHING) {
        fprintf(trace_fp, TRACE_FN_REMAPPED, TRACE_FN_REMAP_EXPR,
                pMac->md_code, pT->td_file, pMac->md_line);
        fprintf(trace_fp, TRACE_FN_REMAP_BASE,
                pT->td_text + pMac->md_name_off);
    }

    pMac->md_code = FTYP_EXPR;
    if (pMac->md_txt_off == 0) {
        pMac->md_res = EMIT_VALUE;

    } else {
        char* pzExpr = pT->td_text + pMac->md_txt_off;
        switch (*pzExpr) {
        case ';':
        case '(':
            pMac->md_res = EMIT_EXPRESSION;
            break;

        case '`':
            pMac->md_res = EMIT_SHELL;
            span_quote(pzExpr);
            break;

        case '"':
        case '\'':
            span_quote(pzExpr);
            /* FALLTHROUGH */

        default:
            pMac->md_res = EMIT_STRING;
        }

        if (OPT_VALUE_TRACE >= TRACE_EVERYTHING)
            fprintf(trace_fp, TRACE_UNKNOWN_FMT, pMac->md_res, pzExpr);
    }

    return mFunc_Expr(pT, pMac);
}


/*=macfunc BOGUS
 *
 *  what:  Out-of-context or unknown functions are bogus.
 *  handler_proc:
 *  load_proc:
 *  unnamed:
=*/
macro_t*
mFunc_Bogus(templ_t* pT, macro_t* pMac)
{
    char * pz = aprf(FN_BOGUS_FMT, pMac->md_code,
                     (pMac->md_code < FUNC_CT)
                     ? apzFuncNames[ pMac->md_code ]
                     : FN_BOGUS_HUH);

    AG_ABEND_IN(pT, pMac, pz);
    /* NOTREACHED */
    return pMac;
}


/*=macfunc TEXT
 *
 *  what:  A block of text to be emitted.
 *  handler_proc:
 *  unnamed:
=*/
macro_t*
mFunc_Text(templ_t* pT, macro_t* pMac)
{
    fputs(pT->td_text + pMac->md_txt_off, cur_fpstack->stk_fp);
    fflush(cur_fpstack->stk_fp);
    return pMac + 1;
}


/*=macfunc COMMENT
 *
 *  what:  A block of comment to be ignored
 *  load_proc:
 *  alias: "#"
 *
 *  desc:
 *    This function can be specified by the user, but there will
 *    never be a situation where it will be invoked at emit time.
 *    The macro is actually removed from the internal representation.
 *
 *    If the native macro name code is @code{#}, then the
 *    entire macro function is treated as a comment and ignored.
 *
 *    @example
 *    [+ # say what you want, but no '+' before any ']' chars +]
 *    @end example
=*/
macro_t *
mLoad_Comment(templ_t * tpl, macro_t * mac, char const ** p_scan)
{
    (void)tpl;
    (void)p_scan;
    memset((void*)mac, 0, sizeof(*mac));
    return mac;
}


/*
 *  mLoad_Unknown  --  the default (unknown) load function
 *
 *  Move any text into the text offset field.
 *  This is used as the default load mechanism.
 */
macro_t *
mLoad_Unknown(templ_t * pT, macro_t * pMac, char const ** p_scan)
{
    char *        pzCopy = pT->td_scan;
    char const *  pzSrc;
    size_t        src_len = (size_t)pMac->md_res; /* macro len  */
    (void)p_scan;

    if (src_len <= 0)
        goto return_emtpy_expr;

    pzSrc = (char const*)pMac->md_txt_off; /* macro text */

    switch (*pzSrc) {
    case ';':
        /*
         *  Strip off scheme comments
         */
        do  {
            while (--src_len, (*++pzSrc != NL)) {
                if (src_len <= 0)
                    goto return_emtpy_expr;
            }

            while (--src_len, IS_WHITESPACE_CHAR(*++pzSrc)) {
                if (src_len <= 0)
                    goto return_emtpy_expr;
            }
        } while (*pzSrc == ';');
        break;

    case '[':
    case '.':
    {
        size_t remLen;

        /*
         *  We are going to recopy the definition name,
         *  this time as a canonical name (i.e. with '[', ']' and '.'
         *  characters and all blanks squeezed out)
         */
        pzCopy = pT->td_text + pMac->md_name_off;

        /*
         *  Move back the source pointer.  We may have skipped blanks,
         *  so skip over however many first, then back up over the name.
         */
        while (IS_WHITESPACE_CHAR(pzSrc[-1]))
            pzSrc--, src_len++;
        remLen   = strlen(pzCopy);
        pzSrc   -= remLen;
        src_len += remLen;

        /*
         *  Now copy over the full canonical name.  Check for errors.
         */
        remLen = canonical_name(pzCopy, pzSrc, (int)src_len);
        if (remLen > src_len)
            AG_ABEND_IN(pT, pMac, LD_UNKNOWN_INVAL_DEF);

        pzSrc  += src_len - remLen;
        src_len = remLen;

        pT->td_scan = pzCopy + strlen(pzCopy) + 1;
        if (src_len <= 0)
            goto return_emtpy_expr;
        break;
    }
    }

    /*
     *  Copy the expression
     */
    pzCopy       = pT->td_scan; /* next text dest   */
    pMac->md_txt_off = (pzCopy - pT->td_text);
    pMac->md_res = 0;
    memcpy(pzCopy, pzSrc, src_len);
    pzCopy      += src_len;
    *(pzCopy++)  = NUL;
    *pzCopy      = NUL; /* double terminate */
    pT->td_scan  = pzCopy;

    return pMac + 1;

 return_emtpy_expr:
    pMac->md_txt_off = 0;
    pMac->md_res    = 0;
    return pMac + 1;
}


/**
 *  Some functions are known to AutoGen, but invalid out of context.
 *  For example, ELIF, ELSE and ENDIF are all known to AutoGen.
 *  However, the load function pointer for those functions points
 *  here, until an "IF" function is encountered.
 */
macro_t*
mLoad_Bogus(templ_t* pT, macro_t* pMac, char const ** p_scan)
{
    char const * pzSrc = (char const*)pMac->md_txt_off; /* macro text */
    char const * pzMac;

    char z[ 64 ];
    (void)p_scan;

    if (pzSrc != NULL) {
        z[0] = ':';
        z[1] = z[2] = ' ';
        strncpy(z+3, pzSrc, (size_t)60);
        z[63] = NUL;
        pzSrc = z;
    }
    else
        pzSrc = zNil;

    {
        int ix = pMac->md_code;
        if ((unsigned)ix >= FUNC_CT)
            ix = 0;

        pzMac = apzFuncNames[ ix ];
    }

    pzSrc = aprf(LD_BOGUS_UNKNOWN, pT->td_file, pMac->md_line, pzMac, pzSrc);

    AG_ABEND_IN(pT, pMac, pzSrc);
    /* NOTREACHED */
    return NULL;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/functions.c */
