
/*
 *  $Id: nested.c,v 4.1 2005/02/20 02:15:48 bkorb Exp $
 *  Time-stamp:      "2005-02-19 17:00:37 bkorb"
 *
 *   Automated Options Nested Values module.
 */

/*
 *  Automated Options copyright 1992-2005 Bruce Korb
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


/*=export_func  optionLoadNested
 * private:
 *
 * what:  parse a hierarchical option argument
 * arg:   + tOptDesc*       + pOptDesc + the descriptor for this arg +
 * arg:   + tOptionLoadMode + mode     + the value formation mode    +
 *
 * doc:
 *  A nested value needs to be parsed
=*/
void
optionLoadNested( tOptDesc* pOD, tOptionLoadMode mode )
{
    return;
}

/*=export_func  optionNestedVal
 * private:
 *
 * what:  parse a hierarchical option argument
 * arg:   + tOptions* + pOpts    + program options descriptor +
 * arg:   + tOptDesc* + pOptDesc + the descriptor for this arg +
 *
 * doc:
 *  Nested value was found on the command line
=*/
void
optionNestedVal( tOptions* pOpts, tOptDesc* pOD )
{
    optionLoadNested( pOD, OPTION_LOAD_UNCOOKED );
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/nested.c */
