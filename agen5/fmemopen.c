/*-
 * Copyright (c) 2004
 *	Bruce Korb.  All rights reserved.
 *
 * This code was inspired from software written by
 *   Hanno Mueller, kontakt@hanno.de
 * and completely rewritten by Bruce Korb, bkorb@gnu.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef ENABLE_FMEMOPEN

/*=--subblock=arg=arg_type,arg_name,arg_desc =*/
/*=*
 * library:  fmem
 * header:   libfmem.h
 *
 * lib_description:
 *
 *  This library only functions in the presence of GNU or BSD's libc.
 *  It is distributed under the Berkeley Software Distribution License.
 *  This library can only function if there is a libc-supplied mechanism
 *  for fread/fwrite/fseek/fclose to call into this code.  GNU uses
 *  fopencookie and BSD supplies funopen.
=*/
/*
 * fmemopen() - "my" version of a string stream
 * inspired by the glibc version, but completely rewritten and
 * extended by Bruce Korb - bkorb@gnu.org
 *
 * - See the man page.
 */

#if defined(__linux)
#  define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux)
#  include <libio.h>
   typedef _IO_off64_t  fmem_off_t;
   typedef int          seek_pos_t;

#elif defined(__bsd)
#  include "local.h"
   typedef size_t  fmem_off_t;
   typedef fpos_t  seek_pos_t;

   typedef int     (cookie_close_function_t)(void *);
   typedef int     (cookie_read_function_t )(void *, char *, int);
   typedef fpos_t  (cookie_seek_function_t )(void *, fpos_t, int);
   typedef int     (cookie_write_function_t)(void *, const char *, int);

#else
#  error  OOPS
#endif

#define PROP_TABLE \
_Prop_( read,       "Read from buffer"        ) \
_Prop_( write,      "Write to buffer"         ) \
_Prop_( append,     "Append to buffer okay"   ) \
_Prop_( binary,     "byte data - not string"  ) \
_Prop_( create,     "allocate the string"     ) \
_Prop_( truncate,   "start writing at start"  ) \
_Prop_( allocated,  "we allocated the buffer" )

#ifndef NUL
# define NUL '\0'
#endif

#define _Prop_(n,s)   BIT_ID_ ## n,
typedef enum { PROP_TABLE BIT_CT } fmem_flags_e;
#undef  _Prop_

#define FLAG_BIT(n)  (1 << BIT_ID_ ## n)

typedef unsigned long mode_bits_t;
typedef unsigned char buf_chars_t;

typedef struct fmem_cookie_s fmem_cookie_t;
struct fmem_cookie_s {
    mode_bits_t    mode;
    buf_chars_t   *buffer;
    size_t         buf_size;    /* Full size of buffer */
    size_t         next_ix;     /* Current position */
    size_t         high_water;  /* Highwater mark of valid data */
    size_t         pg_size;     /* size of a memory page.
                                   Future architectures allow it to vary
                                   by memory region. */
};

static int
fmem_getpMode( const char *pMode, mode_bits_t *pRes )
{
    if (pMode == NULL)
        return 1;

    switch (*pMode) {
    case 'a': *pRes = FLAG_BIT(write) | FLAG_BIT(create) | FLAG_BIT(append);
              break;
    case 'w': *pRes = FLAG_BIT(write) | FLAG_BIT(create) | FLAG_BIT(truncate);
              break;
    case 'r': *pRes = FLAG_BIT(read);
              break;
    default:  return EINVAL;
    }

    /*
     *  If someone wants to supply a "wxxbxbbxbb+" pMode string, I don't care.
     */
    for (;;) {
        switch (*++pMode) {
        case '+': *pRes |= FLAG_BIT(append) | FLAG_BIT(read) | FLAG_BIT(write);
                  if (*pMode != NUL)
                      return EINVAL;
                  break;
        case NUL: break;
        case 'b': *pRes |= FLAG_BIT(binary); continue;
        case 'x': continue;
        default:  return EINVAL;
        }
        break;
    }

    /*
     *  Either the read or write bit must be set. (both okay, too)
     */
    return (*pRes & (FLAG_BIT(read) | FLAG_BIT(write)))
           ? 0 : EINVAL;
}

static int
fmem_extend( fmem_cookie_t *pFMC, size_t new_size )
{
    size_t ns = (new_size + (pFMC->pg_size - 1)) & (~(pFMC->pg_size - 1));

    /*
     *  Though you may extend a file you are writing to, we put a minor
     *  twist on meanings here:  the append bit must be set to extend the data.
     *  This bit is set with either "a"ppend mode in the open mode string, or
     *  with a '+' flag setting read and write mode.
     */
    if (pFMC->mode & FLAG_BIT(append) == 0)
        return -1;

    if (pFMC->mode & FLAG_BIT(allocated) == 0) {
        /*
         *  Previously, this was a user supplied buffer.  We now move to one
         *  of our own.  The user is responsible for the earlier memory.
         */
        void* bf = malloc( ns );
        if (bf == NULL) {
            errno = ENOSPC;
            return -1;
        }
        memcpy( bf, pFMC->buffer, pFMC->buf_size );
        pFMC->buffer = bf;
        pFMC->mode  |= FLAG_BIT(allocated);
    }
    else {
        void* bf = realloc( pFMC->buffer, ns );
        if (bf == NULL) {
            errno = ENOSPC;
            return -1;
        }
        pFMC->buffer = bf;
    }

    /*
     *  Unallocated file space is set to zeros.  Emulate that.
     */
    memset( pFMC->buffer + pFMC->buf_size, 0, ns - pFMC->buf_size );
    pFMC->buf_size = ns;
    return 0;
}

static ssize_t
fmem_read( void *cookie, void *pBuf, size_t sz )
{
    fmem_cookie_t *pFMC = cookie;

    if (pFMC->next_ix + sz > pFMC->buf_size) {
        if (pFMC->next_ix >= pFMC->buf_size)
            return (sz > 0) ? -1 : 0;
        sz = pFMC->buf_size - pFMC->next_ix;
    }

    memcpy( pBuf, pFMC->buffer + pFMC->next_ix, sz );

    pFMC->next_ix += sz;
    if (pFMC->next_ix > pFMC->high_water)
        pFMC->high_water = pFMC->next_ix;

    return sz;
}


static ssize_t
fmem_write( void *cookie, const void *pBuf, size_t sz )
{
    fmem_cookie_t *pFMC = cookie;
    int add_nul_char;

    /*
     *  Only add a NUL character if:
     *
     *  * we are not in binary mode
     *  * there are data to write
     *  * the last character to write is not NUL
     */
    add_nul_char =
           ((pFMC->mode & FLAG_BIT(binary)) != 0)
        && (sz > 0)
        && (((char*)pBuf)[sz - 1] != NUL);

    {
        size_t next_pos = pFMC->next_ix + sz + add_nul_char;
        if (next_pos > pFMC->buf_size) {
            if (fmem_extend( pFMC, next_pos ) != 0) {
                /*
                 *  We could not extend the memory.  Try to write some data.
                 *  Fail if we are either at the end or not writing data.
                 */
                if ((pFMC->next_ix >= pFMC->buf_size) || (sz == 0))
                    return -1; /* no space at all.  errno is set. */

                /*
                 *  Never add the NUL for a truncated write.  "sz" may be
                 *  unchanged or limited here.
                 */
                add_nul_char = 0;
                sz = pFMC->buf_size - pFMC->next_ix;
            }
        }
    }

    memcpy( pFMC->buffer + pFMC->next_ix, pBuf, sz);

    pFMC->next_ix += sz;

    /*
     *  Check for new high water mark and remember it.  Add a NUL if
     *  we do that and if we have a new high water mark.
     */
    if (pFMC->next_ix > pFMC->high_water) {
        pFMC->high_water = pFMC->next_ix;
        if (add_nul_char)
            pFMC->buffer[ pFMC->high_water ] = NUL;
    }

    return sz;
}


static seek_pos_t
fmem_seek (void *cookie, fmem_off_t *p, int dir)
{
    fmem_off_t new_pos;
    fmem_cookie_t *pFMC = cookie;

    switch (dir) {
    case SEEK_SET: new_pos = *p;  break;
    case SEEK_CUR: new_pos = pFMC->next_ix  + *p;  break;
    case SEEK_END: new_pos = pFMC->buf_size - *p;  break;

    case FMEM_IOCTL_BUF_ADDR:
        *(char**)p_offset = pFMC->buffer;
        return 0;

    case FMEM_IOCTL_SAVE_BUF:
        pFMC->mode.allocated = 0;
        return 0;

    default:       goto seek_oops;
    }

    if ((signed)new_pos < 0)
        goto seek_oops;

    if (new_pos > pFMC->buf_size) {
        if (fmem_extend( pFMC, new_pos ))
            return -1; /* errno is set */
    }

    pFMC->next_ix = new_pos;
    return new_pos;

 seek_oops:
    errno = EINVAL;
    return -1;
}


static int
fmem_close( void *cookie )
{
    fmem_cookie_t *pFMC = cookie;

    if (pFMC->mode & FLAG_BIT(allocated))
        free( pFMC->buffer );
    free( pFMC );

    return 0;
}

/*=export_func fmem_ioctl
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
 *  The commands supported are:
 *
 *  @table @code
 *  @item FMEM_IOCTL_BUF_ADDR
 *    Retrieve the address of the buffer.  Future output to the stream
 *    might cause this buffer to be freed and the contents copied to another
 *    buffer.  You must ensure that either you have saved the buffer
 *    (see @code{FMEM_IOCTL_SAVE_BUF} below), or do not do any more I/O to
 *    it while you are using this address.
 *
 *  @item FMEM_IOCTL_SAVE_BUF
 *    Do not deallocate the buffer on close.  You would likely want to use this
 *    after writing all the output data and just before closing.  Otherwise,
 *    the buffer might get relocated.  Once you have specified this, the
 *    current buffer becomes the client program's resposibility to
 *    @code{free()}.
 *  @end table
 *
 *  The third argument is never optional and must be a pointer to where data
 *  are to be retrieved or stored.  It may be NULL if there are no data to
 *  transfer.
=*/
int
fmem_ioctl( FILE* fp, int req, void* ptr )
{
    if (sizeof(long) == sizeof(char*))
        return fseek( fp, (long)ptr, req );
    errno = EINVAL;
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
 *  err:  NULL is returned and errno is set to @code{EINVAL} or @code{ENOSPC}.
 *
 *  doc:
 *
 *  If @code{buf} is @code{NULL}, then a buffer is allocated.
 *  It is allocated to size @code{len}, unless that is zero.
 *  If @code{len} is zero, then @code{getpagesize()} is used and the buffer
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
 *  start at the first NUL character in the initial buffer (or the end of
 *  the buffer if there is no NUL character).
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
fmemopen(void *buf, size_t len, const char *mode)
{
    fmem_cookie_t *pFMC;

    {
        mode_bits_t mode;

        if (fmem_getmode(mode, &mode) != 0) {
            errno = EINVAL;
            return NULL;
        }

        pFMC = malloc(sizeof(fmem_cookie_t));
        if (pFMC == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        pFMC->mode = mode;
    }

    if (buf == NULL)
        pFMC->mode |= FLAG_BIT(allocated);
    pFMC->pg_size = getpagesize();

    if ((pFMC->mode & FLAG_BIT(allocated)) == 0) {
        char *p = pFMC->buffer = buf;

        /*
         *  User allocated buffer.  User responsible for disposal.
         *  IF writing but not appending, start at beginning
         */
        pFMC->next_ix = 0;
        if (  (pFMC->mode & (FLAG_BIT(write) | FLAG_BIT(append)))
           == FLAG_BIT(write) )  {
            pFMC->buffer[0] = NUL;
        } else {
            while ((*p != NUL) && (++(pFMC->next_ix) < len))  p++;
        }
    }

    else if ((pFMC->mode & FLAG_BIT(write)) == 0) {
        /*
         *  No user supplied buffer and we are reading.  Nonsense.
         */
        errno = EINVAL;
        free( pFMC );
        return NULL;
    }

    else {
        /*
         *  We must allocate the buffer.  Whenever we allocate something
         *  beyond what is specified (zero, in this case), the mode had
         *  best include "append".
         */
        if (len == 0) {
            if (pFMC->mode & FLAG_BIT(append) == 0) {
                errno = EINVAL;
                free( pFMC );
                return NULL;
            }

            len = pFMC->pg_size;
        }

        /*
         *  Unallocated file space is set to zeros.  Emulate that.
         */
        pFMC->buffer = calloc(1, len);
        if (pFMC->buffer == NULL) {
            errno = ENOMEM;
            free( pFMC );
            return NULL;
        }

        pFMC->next_ix = 0;
    }

    pFMC->buf_size   = len;
    pFMC->high_water = (pFMC->mode & FLAG_BIT(binary))
                   ? len : strlen(pFMC->buffer);

#if defined(HAVE_FOPENCOOKIE)
    {
        cookie_io_functions_t iof;
        iof.read  = (cookie_read_function_t* )fmem_read;
        iof.write = (cookie_write_function_t*)fmem_write;
        iof.seek  = (cookie_seek_function_t* )fmem_seek;
        iof.close = (cookie_close_function_t*)fmem_close;

        return fopencookie( pFMC, mode, iof );
    }
#elif defined(HAVE_FUNOPEN)
    return funopen( pFMC,
                    (cookie_read_function_t* )fmem_read,
                    (cookie_write_function_t*)fmem_write,
                    (cookie_seek_function_t* )fmem_seek,
                    (cookie_close_function_t*)fmem_close );
#else
#  error We have neither fopencookie(3GNU) nor funopen(3BSD)
#endif
}
#endif /* ENABLE_FMEMOPEN */
