
/*
 *  scmStrings.c
 *  $Id$
 *  Temporary SCM strings.
 *
 * Time-stamp:        "2007-07-04 11:28:25 bkorb"
 *
 * This file is part of AutoGen.
 * AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
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

typedef struct string_buf_s string_buf_t;

struct string_buf_s {
    string_buf_t*   next_p;
    size_t          sb_size;
    size_t          sb_off;
    unsigned char   sb_buf[1];
};

static string_buf_t* ag_strbufs = NULL;

void  ag_scmStrings_init( void )
{
    ag_strbufs = NULL;
}


void  ag_scmStrings_deinit( void )
{
    string_buf_t* sb = ag_strbufs;
    ag_strbufs = NULL;

    while (sb != NULL) {
        string_buf_t* sb_next_p = sb->next_p;
        free( sb );
        sb = sb_next_p;
    }
}


void  ag_scmStrings_free(   void )
{
    string_buf_t* sb = ag_strbufs;

    while (sb != NULL) {
        sb->sb_off = 0;
        sb = sb->next_p;
    }
}


char*
ag_scribble( size_t size )
{
    string_buf_t* sb = ag_strbufs;
    string_buf_t** sb_p = &(ag_strbufs);
    char* buf;

    for (;;) {
        if (sb == NULL) {
            size_t a_size = size + 1 + sizeof(*sb) + 0x2000;
            a_size &= ~0x1FFF;
            *sb_p = sb  = AGALOC( a_size, "SCM String Buffer" );
            sb->next_p  = NULL;
            sb->sb_size = a_size - sizeof(*sb);
            sb->sb_off  = 0;
            break;
        }

        if ((sb->sb_size - sb->sb_off) > size)
            break;

        sb_p = &(sb->next_p);
        sb   = sb->next_p;
    }

    buf = (char*)(sb->sb_buf + sb->sb_off);
    sb->sb_off += size + 1;
    return buf;
}


#if GUILE_VERSION >= 107000

char*
ag_scm2zchars( SCM s, tCC* type )
{
    size_t len = scm_c_string_length(s);
    char* buf;

    if (len == 0) {
        static char z = NUL;
        return &z;
    }

    buf = ag_scribble( len+2 );

    {
        size_t buflen = scm_to_locale_stringbuf( s, buf, len );
        if (buflen != len)
            AG_ABEND( aprf("scm_string_length returned wrong value: "
                           "%d != %d\n", buflen, len));
        buf[ len ] = NUL;
    }

    return buf;
}
#endif

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/autogen.h */
