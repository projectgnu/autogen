
/*
 *  sort.c  $Id: sort.c,v 3.3 2003/11/23 05:21:21 bkorb Exp $
 *
 *  This module implements argument sorting.
 */

/*
 *  Automated Options copyright 1992-2003 Bruce Korb
 *
 *  Automated Options is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Automated Options is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Automated Options.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *
 * As a special exception, Bruce Korb gives permission for additional
 * uses of the text contained in his release of AutoOpts.
 *
 * The exception is that, if you link the AutoOpts library with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the AutoOpts library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Bruce Korb under
 * the name AutoOpts.  If you copy code from other sources under the
 * General Public License into a copy of AutoOpts, as the General Public
 * License permits, the exception does not apply to the code that you add
 * in this way.  To avoid misleading anyone as to the status of such
 * modified files, you must delete this exception notice from them.
 *
 * If you write modifications of your own for AutoOpts, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 */

LOCAL void
optionSort( tOptions* pOpts )
{
    char** ppzOpts;
    char** ppzOpds;
    int    optsIdx = 0;
    int    opdsIdx = 0;

    tOptState os = { NULL, OPTST_DEFINED, TOPT_UNDEFINED, 0, NULL };

    errno = ENOENT;

    /*
     *  If all arguments are named, we can't sort 'em.  There are no operands.
     */
    if (NAMED_OPTS(pOpts))
        return;

    /*
     *  Make sure we can allocate two full-sized arg vectors.
     */
    ppzOpts = malloc( pOpts->origArgCt * sizeof( char* ));
    if (ppzOpts == NULL)
        goto exit_no_mem;

    ppzOpds = malloc( pOpts->origArgCt * sizeof( char* ));
    if (ppzOpds == NULL) {
        free( ppzOpts );
        goto exit_no_mem;
    }

    /*
     *  Now, process all the options from our current position onward.
     *  (This allows interspersed options and arguments for the few
     *  non-standard programs that require it.)
     */
    for (;;) {
        char* pzArg;
        tSuccess res;

        /*
         *  If we're out of arguments, we're done.  Join the option and
         *  operand lists into the original argument vector.
         */
        if (pOpts->curOptIdx >= pOpts->origArgCt) {
            errno = 0;
            goto joinLists;
        }

        pzArg = pOpts->origArgVect[ pOpts->curOptIdx ];
        if (*pzArg != '-') {
            ppzOpds[ opdsIdx++ ] = pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
            continue;
        }

        switch (pzArg[1]) {
        case NUL:
            /*
             *  A regular option.  Put it on the operand list.
             */
            ppzOpds[ opdsIdx++ ] = pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
            continue;

        case '-':
            /*
             *  Two consecutive hypens.  Put them on the options list and then
             *  _always_ force the remainder of the arguments to be operands.
             */
            if (pzArg[2] == NUL) {
                ppzOpts[ optsIdx++ ] =
                    pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                goto restOperands;
            }
            res = longOptionFind( pOpts, pzArg+2, &os );
            break;

        default:
            /*
             *  If short options are not allowed, then do long
             *  option processing.  Otherwise the character must be a
             *  short (i.e. single character) option.
             */
            if ((pOpts->fOptSet & OPTPROC_SHORTOPT) == 0) {
                res = longOptionFind( pOpts, pzArg+1, &os );
            } else {
                res = shortOptionFind( pOpts, pzArg[1], &os );
            }
            break;
        }
        if (FAILED( res )) {
            errno = EIO;
            goto freeTemps;
        }

        /*
         *  We've found an option.  Add the argument to the option list.
         *  Next, we have to see if we need to pull another argument to be
         *  used as the option argument.
         */
        ppzOpts[ optsIdx++ ] = pOpts->origArgVect[ (pOpts->curOptIdx)++ ];

        switch (os.pOD->optArgType) {
        case ARG_MUST:
            /*
             *  An option argument is required.  Long options can either have
             *  a separate command line argument, or an argument attached by
             *  the '=' character.  Figure out which.
             */
            switch (os.optType) {
            case TOPT_SHORT:
                /*
                 *  See if an arg string follows the flag character.  If not,
                 *  the next arg must be the option argument.
                 */
                if (pzArg[2] == NUL) {
                    if (pOpts->curOptIdx >= pOpts->origArgCt) {
                        errno = EIO;
                        goto freeTemps;
                    }
                    ppzOpts[ optsIdx++ ] =
                        pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                }
                break;

            case TOPT_LONG:
                /*
                 *  See if an arg string has already been assigned (glued on
                 *  with an `=' character).  If not, the next is the opt arg.
                 */
                if (os.pzOptArg == (char*)NULL) {
                    if (pOpts->curOptIdx >= pOpts->origArgCt) {
                        errno = EIO;
                        goto freeTemps;
                    }
                    ppzOpts[ optsIdx++ ] =
                        pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                }
                break;
            }
            break;

        case ARG_MAY:
            /*
             *  An option argument is optional.
             */
            switch (os.optType) {
            case TOPT_SHORT:
                /*
                 *  IF nothing is glued on after the current flag character,
                 *  THEN see if there is another argument.  If so and if it
                 *  does *NOT* start with a hyphen, then it is the option arg.
                 */
                if (pzArg[2] == NUL) {
                    if (pOpts->curOptIdx >= pOpts->origArgCt) {
                        errno = 0;
                        goto joinLists;
                    }

                    pzArg = pOpts->origArgVect[ pOpts->curOptIdx ];
                    if (*pzArg != '-')
                        ppzOpts[ optsIdx++ ] =
                            pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                }
                break;

            case TOPT_LONG:
                /*
                 *  Look for an argument if we don't already have one (glued on
                 *  with a `=' character)
                 */
                if (os.pzOptArg == (char*)NULL) {
                    if (pOpts->curOptIdx >= pOpts->origArgCt) {
                        errno = 0;
                        goto joinLists;
                    }

                    pzArg = pOpts->origArgVect[ pOpts->curOptIdx ];
                    if (*pzArg != '-')
                        ppzOpts[ optsIdx++ ] =
                            pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                }
                break;
            }
            continue;

        default: /* CANNOT */
            /*
             *  No option argument.  If we do not have a short option here,
             *  then keep scanning for short options until we get to the end
             *  of the string.
             */
            if (os.optType != TOPT_SHORT)
                continue;

            pzArg += 2;
            while (*pzArg != NUL) {
                if (FAILED( shortOptionFind( pOpts, *pzArg, &os ))) {
                    errno = EIO;
                    goto freeTemps;
                }
                switch (os.pOD->optArgType) {
                case ARG_MUST:
                    /*
                     *  IF we need another argument, be sure it is there and
                     *  take it.
                     */
                    if (pzArg[1] == NUL) {
                        if (pOpts->curOptIdx >= pOpts->origArgCt) {
                            errno = EIO;
                            goto freeTemps;
                        }
                        ppzOpts[ optsIdx++ ] =
                            pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                    }
                    goto shortOptsDone;

                case ARG_MAY:
                    /*
                     *  Take an argument if it is not attached and it does not
                     *  start with a hyphen.
                     */
                    if (pzArg[1] != NUL)
                        goto shortOptsDone;
                    pzArg = pOpts->origArgVect[ pOpts->curOptIdx ];
                    if (*pzArg != '-')
                        ppzOpts[ optsIdx++ ] =
                            pOpts->origArgVect[ (pOpts->curOptIdx)++ ];
                    goto shortOptsDone;

                default:
                    pzArg++;
                }
            } shortOptsDone:;
        }
    }

 restOperands:
    while (pOpts->curOptIdx < pOpts->origArgCt)
        ppzOpds[ opdsIdx++ ] = pOpts->origArgVect[ (pOpts->curOptIdx)++ ];

 joinLists:
    if (optsIdx > 0)
        memcpy( pOpts->origArgVect + 1, ppzOpts, optsIdx * sizeof( char* ));
    if (opdsIdx > 0)
        memcpy( pOpts->origArgVect + 1 + optsIdx,
                ppzOpds, opdsIdx * sizeof( char* ));

 freeTemps:
    free( ppzOpts );
    free( ppzOpds );
    return;

 exit_no_mem:
    errno = ENOMEM;
    return;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/usage.c */
