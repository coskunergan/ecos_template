/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#include "ecos.h"
#include "list.h"



struct list {
  struct list *next;
};

/*---------------------------------------------------------------------------*/
void list_init(list_t list)
{
  *list = NULL;
}
/*---------------------------------------------------------------------------*/
void * list_head(list_t list)
{
  return *list;
}
/*---------------------------------------------------------------------------*/
void list_copy(list_t dest, list_t src)
{
  *dest = *src;
}
/*---------------------------------------------------------------------------*/
void * list_tail(list_t list)
{
  struct list *l;

  if(*list == NULL) {
    return NULL;
  }

  for(l = *list; l->next != NULL; l = l->next);

  return l;
}
/*---------------------------------------------------------------------------*/
void list_add(list_t list, void *item)
{
  struct list *l;

  list_remove(list, item);

  ((struct list *)item)->next = NULL;

  l = list_tail(list);

  if(l == NULL) {
    *list = item;
  } else {
    l->next = item;
  }
}
/*---------------------------------------------------------------------------*/
void list_push(list_t list, void *item)
{
  list_remove(list, item);

  ((struct list *)item)->next = *list;
  *list = item;
}
/*---------------------------------------------------------------------------*/
void * list_chop(list_t list)
{
  struct list *l, *r;

  if(*list == NULL) {
    return NULL;
  }
  if(((struct list *)*list)->next == NULL) {
    l = *list;
    *list = NULL;
    return l;
  }

  for(l = *list; l->next->next != NULL; l = l->next);

  r = l->next;
  l->next = NULL;

  return r;
}
/*---------------------------------------------------------------------------*/
void * list_pop(list_t list)
{
  struct list *l;
  l = *list;
  if(*list != NULL) {
    *list = ((struct list *)*list)->next;
  }

  return l;
}
/*---------------------------------------------------------------------------*/
void list_remove(list_t list, void *item)
{
  struct list *l, *r;

  if(*list == NULL) {
    return;
  }

  r = NULL;
  for(l = *list; l != NULL; l = l->next) {
    if(l == item) {
      if(r == NULL) {

	*list = l->next;
      } else {

	r->next = l->next;
      }
      l->next = NULL;
      return;
    }
    r = l;
  }
}
/*---------------------------------------------------------------------------*/
int list_length(list_t list)
{
  struct list *l;
  int n = 0;

  for(l = *list; l != NULL; l = l->next) {
    ++n;
  }

  return n;
}
/*---------------------------------------------------------------------------*/
void list_insert(list_t list, void *previtem, void *newitem)
{
  if(previtem == NULL) {
    list_push(list, newitem);
  } else {

    ((struct list *)newitem)->next = ((struct list *)previtem)->next;
    ((struct list *)previtem)->next = newitem;
  }
}
/*---------------------------------------------------------------------------*/
void * list_item_next(void *item)
{
  return item == NULL? NULL: ((struct list *)item)->next;
}
/*---------------------------------------------------------------------------*/
/** @} */
