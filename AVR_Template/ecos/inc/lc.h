/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifdef DOXYGEN

#define LC_INIT(lc)
#define LC_SET(lc)
#define LC_RESUME(lc)
#define LC_END(lc)

#endif /* DOXYGEN */

#ifndef __LC_H__
#define __LC_H__


#ifdef LC_INCLUDE
#include LC_INCLUDE
#else
#include "lc-switch.h"
#endif /* LC_INCLUDE */

#endif /* __LC_H__ */

/** @} */
