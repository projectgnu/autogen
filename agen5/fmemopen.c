
/*=--subblock=arg=arg_type,arg_name,arg_desc =*/
/*=*
 * library:  fmem
 * header:   lib-fmem.h
 *
 * lib_description:
 *
 *  This library only functions in the presence of GNU libc.
 *  It is distributed under the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  The GNU C Library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with the GNU C Library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA.
=*/
/*
 * fmemopen() - "my" version of a string stream
 * Hanno Mueller, kontakt@hanno.de
 * extended by Bruce Korb - bkorb@gnu.org
 *
 * - See the man page.
 */

#ifdef HAVE_FOPENCOOKIE

STATIC int fmem_getmode (const char *mode, fmem_io_mode_t *mptr);

#ifndef __set_errno
#  define __set_errno(e)  errno = e
#endif

STATIC int
fmem_getmode (const char *mode, fmem_io_mode_t *mptr)
{
    unsigned char i;

    if (mode == NULL)
        return 1;

    memset ((void *) mptr, 0, sizeof (*mptr));

    switch (*mode)
    {
    case 'a':
        mptr->extensible = mptr->write = mptr->create = mptr->append = 1;
        break;
    case 'w':
        mptr->write = mptr->create = mptr->truncate = 1;
        break;
    case 'r':
        mptr->read = 1;
        break;
    default:
        return 1;
    }

    for (i = 1; i < 4; ++i)
    {
        switch (*++mode)
        {
        case NUL:
            break;
        case '+':
            mptr->extensible = mptr->read = mptr->write = 1;
            continue;
        case 'b':
            mptr->binary = 1;
            continue;
        case 'x':
            continue;
        default:
            return 1;
        }
        break;
    }

    if (!mptr->read && !mptr->write)
        return 1;

    return 0;
}

STATIC int
fmem_extend (fmem_cookie_t *pFMC, size_t new_size)
{
    void* bf;
    size_t ns = (new_size + (pFMC->pg_size - 1)) & (~(pFMC->pg_size - 1));

    /*
     *  Though you may extend a file you are writing to, we put a minor
     *  twist on meanings here:  the append bit must be set to extend the data.
     *  This bit is set with either "a"ppend mode in the open mode string, or
     *  with a '+' flag setting read and write mode.
     */
    if (pFMC->mode.extensible == 0)
        return -1;

    if (pFMC->mode.allocated == 0)
    {
        /*
         *  Previously, this was a user supplied buffer.  We now move to one
         *  of our own.  The user is responsible for the earlier memory.
         */
        bf = malloc (ns);
        if (bf == NULL)
        {
            __set_errno (ENOSPC);
            return -1;
        }
        memcpy (bf, pFMC->buffer, pFMC->size);
        pFMC->buffer = bf;
        pFMC->mode.allocated = 1;
    }
    else
    {
        bf = realloc (pFMC->buffer, ns);
        if (bf == NULL)
        {
            __set_errno (ENOSPC);
            return -1;
        }
        pFMC->buffer = bf;
    }

    /*
     *  Unallocated file space is set to zeros.  Emulate that.
     */
    memset (pFMC->buffer + pFMC->size, 0, ns - pFMC->size);
    pFMC->size = ns;
    return 0;
}

STATIC ssize_t
fmem_read (void *cookie, void *b, size_t s)
{
    fmem_cookie_t *pFMC = (fmem_cookie_t *) cookie;

    if (pFMC->pos + s > pFMC->size)
    {
        if (pFMC->pos >= pFMC->size)
            return (s > 0) ? -1 : 0;
        s = pFMC->size - pFMC->pos;
    }

    memcpy (b, &(pFMC->buffer[pFMC->pos]), s);

    pFMC->pos += s;
    if (pFMC->pos > pFMC->maxpos)
        pFMC->maxpos = pFMC->pos;

    return s;
}

STATIC ssize_t
fmem_write (void *cookie, const void *b, size_t s)
{
    const char* pBuf = b;
    fmem_cookie_t *pFMC;
    int add_nul_char;

    pFMC = (fmem_cookie_t *) cookie;

    /*
     *  Only add a NUL character if:
     *
     *  * we are not in binary mode
     *  * there are data to write
     *  * the last character to write is not NUL
     */
    add_nul_char = (pFMC->mode.binary == 0)
                   && (  (s == 0)
                      || (pBuf[s - 1] != NUL) );

    if (pFMC->mode.append != 0)
        pFMC->pos = pFMC->maxpos;

    if (  (pFMC->pos + s + add_nul_char > pFMC->size)
          && (fmem_extend (pFMC, pFMC->pos + s + add_nul_char) != 0)  )
    {
        if (pFMC->pos + add_nul_char == pFMC->size)
        {
            __set_errno (ENOSPC);
            return -1;
        }
        s = pFMC->size - pFMC->pos - add_nul_char;
    }

    memcpy (&(pFMC->buffer[pFMC->pos]), b, s);

    pFMC->pos += s;
    if (pFMC->pos > pFMC->maxpos)
    {
        pFMC->maxpos = pFMC->pos;
        if (add_nul_char)
            pFMC->buffer[pFMC->maxpos] = NUL;
    }

    return s;
}

STATIC int
fmem_seek (void *cookie, _IO_off64_t *p, int w)
{
    _IO_off64_t new_pos;
    fmem_cookie_t *pFMC = (fmem_cookie_t *) cookie;

    switch (w)
    {
    case SEEK_SET:
        new_pos = *p;
        break;

    case SEEK_CUR:
        new_pos = pFMC->pos + *p;
        break;

    case SEEK_END:
        new_pos = pFMC->size - *p;
        break;

    case FMEM_IOCTL_BUFFER:
        *(char**)p = pFMC->buffer;
        return 0;

    default:
        __set_errno (EINVAL);
        return -1;
    }

    if (new_pos < 0)
        return -1;

    if (new_pos > pFMC->size)
    {
        if (fmem_extend (pFMC, new_pos))
            return -1;
    }

    *p = pFMC->pos = new_pos;
    return 0;
}

STATIC int
fmem_close (void *cookie)
{
    fmem_cookie_t *pFMC = (fmem_cookie_t *) cookie;

    if (pFMC->mode.allocated)
        free (pFMC->buffer);
    free (pFMC);

    return 0;
}

/*=export_func fmemioctl
 *
 *  what:  get information about a string stream
 *
 *  arg: + FILE* + fptr  + the string stream
 *  arg: + int   + req   + the requested data
 *  arg: + void* + ptr   + ptr to result area
 *
 *  err: non-zero is returned and @code{errno} is set.
 *
 *  doc:
 *
 *  This routine surreptitiously slips in a special request.
 *  Currently, the only special request supported is,
 *  @code{FMEM_IOCTL_BUFFER}.  The third argument is never optional
 *  and must be a pointer to where the buffer address is to be stored.
 *  "@code{sizeof long}" must be the same as "@code{sizeof char*}".
=*/
int
fmemioctl( FILE* fp, int req, void* ptr )
{
    if (sizeof(long) == sizeof(char*))
        return fseek( fp, (long)ptr, req );
    __set_errno (EINVAL);
    return -1;
}

/*=export_func fmemopen
 *
 *  what:  Open a stream to a string
 *
 *  arg: + void*  + buf  + buffer to use for i/o +
 *  arg: + size_t + len  + size of the buffer +
 *  arg: + char*  + mode + mode string, a la fopen(3C) +
 *
 *  ret-type:  FILE*
 *  ret-desc:  a stdio FILE* pointer
 *
 *  err:  NULL is returned and errno is set
 *
 *  doc:
 *
 *  If @code{buf} is @code{NULL}, then a buffer is allocated.
 *  It is allocated to size @code{len}, unless that is zero.
 *  If @code{len} is zero, then @code{pagesize()} is used and the buffer
 *  is marked as "extensible".  Any allocated memory is @code{free()}-ed
 *  when @code{fclose(3C)} is called.
 *
 *  The mode string is interpreted as follows.  If the first character of
 *  the mode is:
 *
 *  @table @code
 *  @item a
 *  Then the string is opened in "append" mode.
 *  Append mode is always extensible.  In binary mode, "appending" will
 *  begin from the end of the initial buffer.  Otherwise, appending will
 *  start at the first non-NUL character in the initial buffer.
 *
 *  @item w
 *  Then the string is opened in "write" mode.
 *  Writing (and any reading) start at the beginning of the buffer.  If the
 *  buffer was supplied by the caller and it is allowed to be extended, then
 *  the initial buffer may or may not be in use at any point in time, and
 *  the user is responsible for handling the disposition of the initial
 *  memory.
 *
 *  @item r
 *  Then the string is opened in "read" mode.
 *  @end table
 *
 *  @noindent
 *  If it is not one of these three, the open fails and @code{errno} is
 *  set to @code{EINVAL}.  These initial characters may be followed by:
 *
 *  @table @code
 *  @item +
 *  The buffer is marked
 *  as extensible and both reading and writing is enabled.
 *
 *  @item b
 *  The I/O is marked as
 *  "binary" and a trailing NUL will not be inserted into the buffer.
 *
 *  @item x
 *  This is ignored.
 *  @end table
 *
 *  @noindent
 *  Any other letters following the inital 'a', 'w' or 'r' will cause an error.
 =*/
FILE *
fmemopen (void *buf, size_t len, const char *mode)
{
    fmem_cookie_t *pFMC = (fmem_cookie_t *) malloc (sizeof (fmem_cookie_t));
    if (pFMC == NULL) {
        __set_errno (ENOMEM);
        return NULL;
    }

    if (fmem_getmode (mode, &pFMC->mode) != 0) {
        __set_errno (EINVAL);
        free (pFMC);
        return NULL;
    }

    pFMC->mode.allocated = ((buf == NULL) || (len == 0));
    pFMC->pg_size = getpagesize ();

    if (pFMC->mode.allocated == 0) {
        char *p = pFMC->buffer = buf;

        /*
         *  IF writing but not appending, start at beginning
         */
        if ((pFMC->mode.write != 0) && (pFMC->mode.append == 0)) {
            pFMC->buffer[0] = NUL;
            pFMC->pos = 0;

        } else if ((pFMC->mode.binary != 0) && (pFMC->mode.append != 0)) {
            pFMC->pos = len;

        } else {
            pFMC->pos = 0;
            while ((*p != NUL) && (++(pFMC->pos) < len))  p++;
        }
    }

    else if (pFMC->mode.write == 0) {
        /*
         *  No user supplied buffer and we are reading.  Nonsense.
         */
        __set_errno (EINVAL);
        free (pFMC);
        return NULL;
    }

    else {
        /*
         *  We must allocate the buffer.  Whenever we allocate something
         *  beyond what is specified (zero, in this case), the mode had
         *  best include "append".
         */
        if (len == 0) {
            if (pFMC->mode.extensible == 0) {
                __set_errno (EINVAL);
                free (pFMC);
                return NULL;
            }

            len = pFMC->pg_size;
        }

        /*
         *  Unallocated file space is set to zeros.  Emulate that.
         */
        pFMC->buffer = (char *) calloc (1, len);
        if (pFMC->buffer == NULL) {
            __set_errno (ENOMEM);
            free (pFMC);
            return NULL;
        }

        pFMC->pos = 0;
    }

    pFMC->size   = len;
    pFMC->maxpos = (pFMC->mode.binary) ? len : strlen (pFMC->buffer);

    {
        cookie_io_functions_t iof;
        iof.read  = (cookie_read_function_t *)fmem_read;
        iof.write = (cookie_write_function_t*)fmem_write;
        iof.seek  = (cookie_seek_function_t *)fmem_seek;
        iof.close = (cookie_close_function_t*)fmem_close;

        return fopencookie (pFMC, mode, iof);
    }
}

#endif /* HAVE_FOPENCOOKIE */

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/fmemopen.c */
