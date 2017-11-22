/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifndef PROCINIT_H_
#define PROCINIT_H_


#define CC_NO_VA_ARGS 0

#if  !CC_NO_VA_ARGS
#define PROCINIT(...)					\
const struct process *procinit[] = {__VA_ARGS__, NULL}
#endif

void procinit_init(void);

#endif /* PROCINIT_H_ */
