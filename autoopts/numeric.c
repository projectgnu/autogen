
/*
 *  $Id: numeric.c,v 2.1 2000/10/17 02:57:00 bkorb Exp $
 *
 *   Automated Options Paged Usage module.
 *
 *  This routine will run run-on options through a pager so the
 *  user may examine, print or edit them at their leisure.
 */

/*
 *  Automated Options copyright 1992-1999 Bruce Korb
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include "autoopts.h"

/*
 *  The value is true, unless it starts with 'n' or 'f' or "#f" or
 *  it is an empty string or it is a number that evaluates to zero
 */
DEF_PROC_2( , void, optionNumericVal,
            tOptions*, pOpts,
            tOptDesc*, pOD )
{
    char* pz;
    long  val;

    if (pOD->pzLastArg == NULL)
	return;

    val = strtol( pOD->pzLastArg, &pz, 0 );
    if (*pz != NUL) {
	fprintf( stderr, "%s error:  `%s' is not a recognizable number\n",
		 pOpts->pzProgName, pOD->pzLastArg );
	(*(pOpts->pUsageProc))(pOpts, EXIT_FAILURE);
    }

    pOD->pzLastArg = (char*)val;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * pgusage.c ends here */