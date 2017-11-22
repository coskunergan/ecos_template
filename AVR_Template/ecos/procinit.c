/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#include "ecos.h"
#include "procinit.h"

#define DEBUG 0


#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif



extern const struct process *procinit[];

/*---------------------------------------------------------------------------*/
void
procinit_init(void)
{
  int i;

  for(i = 0; procinit[i] != NULL; ++i) {
    process_start((struct process *)procinit[i], NULL);
    PRINTF("starting process '%s'\n", (struct process *)procinit[i]->name);
  }

}
/*---------------------------------------------------------------------------*/
