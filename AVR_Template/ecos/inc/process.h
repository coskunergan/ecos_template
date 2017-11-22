/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "pt.h"

// iþlem olay tipi char tanýmlanmýþ böylece 256 adet olay oluþturulabilir.
typedef unsigned char process_event_t;
// iþlem data tipi boþ olarak gösteriliyor data tipi cast edilerek kullanýlmalýdýr.
typedef void *        process_data_t;
// iþlem olay sayýsý char olarak tanýmlanmýþ listede en fazla 256 adet olay biriktirilebilir.
typedef unsigned char process_num_events_t;


#define NULL  0
#define PROCESS_ERR_OK        0
#define PROCESS_ERR_FULL      1

#define PROCESS_NONE          NULL
// olay listesinde max 32 adet olay tutulabilir
#ifndef PROCESS_CONF_NUMEVENTS
#define PROCESS_CONF_NUMEVENTS 32
#endif /* PROCESS_CONF_NUMEVENTS */

#define PROCESS_EVENT_NONE            0x80
#define PROCESS_EVENT_INIT            0x81
#define PROCESS_EVENT_POLL            0x82
#define PROCESS_EVENT_EXIT            0x83
#define PROCESS_EVENT_SERVICE_REMOVED 0x84
#define PROCESS_EVENT_CONTINUE        0x85
#define PROCESS_EVENT_MSG             0x86
#define PROCESS_EVENT_EXITED          0x87
#define PROCESS_EVENT_TIMER           0x88
#define PROCESS_EVENT_COM             0x89
#define PROCESS_EVENT_MAX             0x8a

#define PROCESS_BROADCAST NULL
#define PROCESS_ZOMBIE ((struct process *)0x1)


#define PROCESS_BEGIN()             PT_BEGIN(process_pt)
#define PROCESS_END()               PT_END(process_pt)
#define PROCESS_WAIT_EVENT()        PROCESS_YIELD()
#define PROCESS_WAIT_EVENT_UNTIL(c) PROCESS_YIELD_UNTIL(c)
#define PROCESS_YIELD()             PT_YIELD(process_pt)
#define PROCESS_YIELD_UNTIL(c)      PT_YIELD_UNTIL(process_pt, c)
#define PROCESS_WAIT_UNTIL(c)       PT_WAIT_UNTIL(process_pt, c)
#define PROCESS_WAIT_WHILE(c)       PT_WAIT_WHILE(process_pt, c)
#define PROCESS_EXIT()              PT_EXIT(process_pt)
#define PROCESS_PT_SPAWN(pt, thread)   PT_SPAWN(process_pt, pt, thread)


#define PROCESS_PAUSE()             do {				\
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_CONTINUE, NULL);	\
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);               \
} while(0)


#define PROCESS_POLLHANDLER(handler) if(ev == PROCESS_EVENT_POLL) { handler; }


#define PROCESS_EXITHANDLER(handler) if(ev == PROCESS_EVENT_EXIT) { handler; }

// process thread her kullanýldýðýnda bir PT_THREAD fonksiyonunu extern eder ve
// listede tutacaðý fonksiyon adresi, olayý, data sý için bir struct tanýmlar.
#define PROCESS_THREAD(name, ev, data) 				\
static PT_THREAD(process_thread_##name(struct pt *process_pt,	\
				       process_event_t ev,	\
				       process_data_t data))

#define PROCESS_NAME(name) extern struct process name


#if PROCESS_CONF_NO_PROCESS_NAMES
#define PROCESS(name, strname)				\
  PROCESS_THREAD(name, ev, data);			\
  struct process name = { NULL,		        \
                          process_thread_##name }
#else
#define PROCESS(name, strname)				\
  PROCESS_THREAD(name, ev, data);			\
  struct process name = { NULL, strname,		\
                          process_thread_##name }
#endif

// iþlem listesinin yapýsý
// 1- self referance struct olarak baðlanacaklarý için sonraki iþlemin pointer adresi
// 2- varsa iþlemin string adý yoksa NULL
// 3- iþlemin protothreads fonksiyon adresi
// 4- iþlemin protothreads struct yapýsý
// 5- iþlemin durumu ve poll isteði
struct process
{
    struct process *next;
#if PROCESS_CONF_NO_PROCESS_NAMES
#define PROCESS_NAME_STRING(process) ""
#else
    const char *name;
#define PROCESS_NAME_STRING(process) (process)->name
#endif
    PT_THREAD((* thread)(struct pt *, process_event_t, process_data_t));
    struct pt pt;
    unsigned char state;
    unsigned char needspoll;
};

void process_start(struct process *p, process_data_t data);
int process_post(struct process *p, process_event_t ev, process_data_t data);
void process_post_synch(struct process *p,process_event_t ev, process_data_t data);
void process_exit(struct process *p);

#define PROCESS_CURRENT() process_current

extern struct process *process_current;

#define PROCESS_CONTEXT_BEGIN(p) {\
struct process *tmp_current = PROCESS_CURRENT();\
process_current = p

#define PROCESS_CONTEXT_END(p) process_current = tmp_current; }

process_event_t process_alloc_event(void);
void process_poll(struct process *p);
void process_init(void);
int process_run(void);
int process_is_running(struct process *p);
int process_nevents(void);
extern struct process *process_list;

#define PROCESS_LIST() process_list

#endif /* PROCESS_H_ */
