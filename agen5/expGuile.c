
/*
 *  agExpr.c
 *  $Id: expGuile.c,v 1.1.1.1 1999/10/14 00:33:53 bruce Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
 */

/*
 *  AutoGen copyright 1992-1999 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

#include <string.h>

#include "autogen.h"
#include <guile/gh.h>
#include "expGuile.h"

#ifndef HAVE_STRFTIME
#  include "compat/strftime.c"
#endif



    teGuileType
gh_type_e( SCM typ )
{
    if (gh_boolean_p( typ ))
        return GH_TYPE_BOOLEAN;

    if (gh_symbol_p( typ ))
        return GH_TYPE_SYMBOL;

    if (gh_char_p( typ ))
        return GH_TYPE_CHAR;

    if (gh_vector_p( typ ))
        return GH_TYPE_VECTOR;

    if (gh_pair_p( typ ))
        return GH_TYPE_PAIR;

    if (gh_number_p( typ ))
        return GH_TYPE_NUMBER;

    if (gh_string_p( typ ))
        return GH_TYPE_STRING;

    if (gh_procedure_p( typ ))
        return GH_TYPE_PROCEDURE;

    if (gh_list_p( typ ))
        return GH_TYPE_LIST;

    if (gh_inexact_p( typ ))
        return GH_TYPE_INEXACT;

    if (gh_exact_p( typ ))
        return GH_TYPE_EXACT;

    return GH_TYPE_UNDEFINED;
}


/*=gfunc max
 *
 * req:  0
 * var:  1
 *doc:  Return the maximum value in the list
=*/
    SCM
ag_scm_max( SCM list )
{
    int   len;
    SCM   car;
    long  max_val = LONG_MIN;
#   ifndef MAX
#     define MAX(m,v) ((v > m) ? v : m)
#   endif
    len = scm_ilength( list );
    if (len <= 0)
        return SCM_UNDEFINED;

    while (--len >= 0) {
        long val;

        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        switch (gh_type_e( car )) {
        case GH_TYPE_BOOLEAN:
            if (car == SCM_BOOL_F) {
                val = 0;
            } else {
                val = 1;
            }
            break;

        case GH_TYPE_CHAR:
            val = (int)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            val = gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
        {
            char* pz = SCM_CHARS( car );
            if (! isdigit( *pz ))
                continue;
            val = strtol( pz, (char**)NULL, 0 );
            break;
        }

        default:
            continue;
        }
        max_val = MAX( max_val, val );
    }

#   undef MAX
    return gh_long2scm( max_val );
}


/*=gfunc min
 *
 * req:  0
 * var:  1
 *doc:  Return the minimum value in the list
=*/
    SCM
ag_scm_min( SCM list )
{
    int   len;
    SCM   car;
    long  min_val = LONG_MAX;
#   ifndef MIN
#     define MIN(m,v) ((v < m) ? v : m)
#   endif
    len = scm_ilength( list );
    if (len <= 0)
        return SCM_UNDEFINED;

    while (--len >= 0) {
        long val;

        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        switch (gh_type_e( car )) {
        case GH_TYPE_BOOLEAN:
            if (car == SCM_BOOL_F) {
                val = 0;
            } else {
                val = 1;
            }
            break;

        case GH_TYPE_CHAR:
            val = (int)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            val = gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
        {
            char* pz = SCM_CHARS( car );
            if (! isdigit( *pz ))
                continue;
            val = strtol( pz, (char**)NULL, 0 );
            break;
        }

        default:
            continue;
        }
        min_val = MIN( min_val, val );
    }

#   undef MIN
    return gh_long2scm( min_val );
}


/*=gfunc sum
 *
 * req:  0
 * var:  1
 *doc:  DOC STRING
=*/
    SCM
ag_scm_sum( SCM list )
{
    int  len = scm_ilength( list );
    long sum = 0;

    if (len <= 0)
        return gh_int2scm( 0 );

    do  {
        SCM  car = SCM_CAR( list );
        list = SCM_CDR( list );
        switch (gh_type_e( car )) {
        default:
            return SCM_UNDEFINED;

        case GH_TYPE_CHAR:
            sum += (long)(unsigned char)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            sum += gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
            sum += strtol( SCM_CHARS( car ), (char**)NULL, 0 );
        }
    } while (--len > 0);

    return gh_long2scm( sum );
}
/* end of expGuile.c */
