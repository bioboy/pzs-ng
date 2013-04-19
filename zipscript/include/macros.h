#ifndef _MACROS_H_
#define _MACROS_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#else
#error "No dirent header, quitting."
#endif
#endif

/* No race */
#if ( announce_norace == TRUE )
#define CHOOSE(a,b,c)	g.v.total.users > 1 ? b : c;
#else
#define CHOOSE(a,b,c)	b;
#endif

#ifndef alloca
#define m_alloc(x) malloc(x)
#define m_free(p) free(p)
#else
#define m_alloc(x) alloca(x)
#define m_free(p)
#endif

#endif
