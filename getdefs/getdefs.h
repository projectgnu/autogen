
#ifndef GETDEFS_HEADER
#define GETDEFS_HEADER

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>

#include "opts.h"

#define NUL            '\0'
#define MAX_SUBMATCH   1
#define COUNT(a)       (sizeof(a)/sizeof(a[0]))

#ifdef DEFINE
#  define MODE
#  define VALUE(v) = v
#else
#  define MODE extern
#  define VALUE(v)
#endif

MODE char*    pzDefText VALUE( (char*)NULL );

MODE regex_t  attrib_re;

#endif /* GETDEFS_HEADER */
