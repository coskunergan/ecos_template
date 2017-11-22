/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */
#include "ecos.h"
#include "process.h"

// pointer structure larý
struct process *process_list = NULL; // iþlem listesi seft referance struct þeklinde birbirine baðlanýrlar.
struct process *process_current = NULL; // çalýþmakta olan iþlemin tutulduðu struct

static process_event_t lastevent; // olay sayýsýný tutan tanýmlama

struct event_data   // olay listesi parametereleri
{
    process_event_t ev; // olayýn tanýmý
    process_data_t data;// olayýn datasý eðer olay mesaj göndermek ise iþlemler birbirine data gönderebilir.
    struct process *p; // olayý gerçekleþtirecek olan iþlem
};

// bu iki register olay listesini ring buffer mantýðýnda kullanýlmasýný saðlar
static process_num_events_t nevents; // olay listesine olay eklenmesi artýþýný tutacak adres max kontrolü için
static process_num_events_t fevent;  // olay listesinde iþlenen olay indexinin tutacak adres
// olay dizisinin struct adresinin tanýmlanmasý
static struct event_data events[PROCESS_CONF_NUMEVENTS];

#if PROCESS_CONF_STATS // eger ulaþýlmýþ max olay sayýsýnýn tutulmasý isteniyorsa tanýmlanacak
process_num_events_t process_maxevents; // max olay sayacý tanýmlamasý
#endif

static volatile unsigned char poll_requested; // poll çalýþtýrýlan fonksiyon varsa aktif edilen flag

#define PROCESS_STATE_NONE        0 // iþlemin silinmesi ve ya boþ olmasý durumu
#define PROCESS_STATE_RUNNING     1 // iþlemin hala listede koþacak durum da beklediði durum
#define PROCESS_STATE_CALLED      2 // iþlemin çalýþýr halde oladuðu durum

static void call_process(struct process *p, process_event_t ev, process_data_t data);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
process_event_t process_alloc_event(void)  //  her çaðýrýldýgýnda olay sayýsý artar
{
    return lastevent++;
}
/*---------------------------------------------------------------------------*/
void process_start(struct process *p, process_data_t data) // iþlem basþlatan fonk.
{
    struct process *q; // temp bir struct açýlýr

    // çalýþtýrýlmak istenen iþlem, listede zaten çalýþýyor mu diye kontrol ediliryor
    // listenin sonunda NULL olacaðý için iþlem listesi bitince döngüden çýkar
    for(q = process_list; q != p && q != NULL; q = q->next);

    // eðer iþlem zaten çalýþýyorsa çýkar
    if(q == p)
    {
        return;
    }
    // p->next artýk listenin sonunu iþaret ediyor ve Null dur listenin en baþýndaki iþlem en sonuna baðlanýr.
    p->next = process_list;
    // boþa çýkan liste baþý iþlemi yerine yeni çalýþtýrýlacak iþlem yüklenir.
    process_list = p;
    // durumu koþar olarak deðiþtirilir.
    p->state = PROCESS_STATE_RUNNING;
    // PT nin switch case deðeri sýfýrlanýr.
    PT_INIT(&p->pt);

    // debug comdan bilgi yazdýrýlýr.
    PRINTF("process: starting '%s'\n", PROCESS_NAME_STRING(p));

    // artýk iþlem listeye eklendi çalýþtýrýlmasý için post edilmesi gerekiyor.
    // iþlemin eventine ilk defa çalýþtýðý için kurulacak bilgisi giriliyor.
    // iþlemin datasý aynen aktarýlýyor.
    process_post_synch(p, PROCESS_EVENT_INIT, data);
}
/*---------------------------------------------------------------------------*/
static void exit_process(struct process *p, struct process *fromprocess)
{
    register struct process *q; // temp struct oluþturulur.
    struct process *old_current = process_current;  // þuanda çalýþýr durumda olan baþka bir iþlem varsa
    // bitirilecek iþlem ile ilgili çalýþmalar yapýlana kadar yedekleniyor

    // ekrana bilgi gönderilir
    PRINTF("process: exit_process '%s'\n", PROCESS_NAME_STRING(p));

    /* iþlem listesi kontrol edilip bitirilecek iþlem listede olup olmadýðýna bakýlýyor */
    for(q = process_list; q != p && q != NULL; q = q->next);
    if(q == NULL)
    {
        return; //bulunamadý ise çýkýlýyor. (listede çalýþýr olmayan iþlem bitirilemez)
    }

    if(process_is_running(p))   // eger bitirilece kiþlemin durumu koþar yada çaðrýlmýþ ise girilir.
    {
        // iþlemin durumu yok olarak iþaretlenir
        p->state = PROCESS_STATE_NONE;

        // bitirilecek iþlem listedeki bütün iþlemle çaðýrýlarak HEPSÝNE bildiriliyor.
        for(q = process_list; q != NULL; q = q->next)
        {
            if(p != q)
            {
                // diðer iþlemlere bitirilen iþlemin verisi de data kýsmýndan gönderiliyor.
                // bu sayede birbirine baðlý olarak çalýþan iþlerde kendilerini kapatabilir.
                call_process(q, PROCESS_EVENT_EXITED, (process_data_t)p);
            }
        }
        // eger iþlem kendisi deðilde baþka bir iþlem tarafýndan kapatýlýyorsa girilir.
        if(p->thread != NULL && p != fromprocess)
        {
            // burada iþlem kapanýþ iþlerini yapmasý için son bir kez çaðýrýlýr.
            process_current = p;
            p->thread(&p->pt, PROCESS_EVENT_EXIT, NULL);
        }
    }
    // iþlem listenin ilk üyesimiydi.
    if(p == process_list)
    {
        // ilk üye ezilerek yerine bir sonraki üye eklenir.
        process_list = process_list->next;
    }
    else
    {
        // iþlem listenin neresinde bulunuyor
        for(q = process_list; q != NULL; q = q->next)
        {
            if(q->next == p)
            {
                // iþlem bulundu adresi ezilip yerine bir sonraki iþlem eklenerek bitiriliyor.
                q->next = p->next;
                break;
            }
        }
    }
    // daha önce çalýþýr durumda olan herhangi bir iþlem kaldýgý yerden devam ettiriliyor
    process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void call_process(struct process *p, process_event_t ev, process_data_t data)
{
    int ret; // geri dönüþ degiþkeni tanýmlanýr

#if DEBUG // debug com dan her iþlem çaðýrýldýðýnda bilgi yazdýrýlabilir ve event sayesinde o iþlemin adý ve aþamasý öðrenilebilir.
    if(p->state == PROCESS_STATE_CALLED)
    {
        printf("process: process '%s' called again with event %d\n", PROCESS_NAME_STRING(p), ev);
    }
#endif /* DEBUG */
    // çalýþtýrýlacak iþlem koþuyorsa ve fonk. adresi null deðilse devam edilir.
    if((p->state & PROCESS_STATE_RUNNING) &&
            p->thread != NULL)
    {
        // ekrana bilgilendirme yazýlýr.
        PRINTF("process: calling process '%s' with event %d\n", PROCESS_NAME_STRING(p), ev);
        // çalýþan iþleme adresi yüklenir.
        process_current = p;
        // durumu çaðrýldý olarak deiþtirilir.
        p->state = PROCESS_STATE_CALLED;
        // fonksiyon pointer dan protothreads li yapýda oluþturulmuþ fonksiyon adresine dallanýlýr.
        ret = p->thread(&p->pt, ev, data);
        // iþlem çalýþtýrýldý ve geri dönüldü geri dönüþ bilgisine bakýlýyor
        if(ret == PT_EXITED ||
                ret == PT_ENDED ||
                ev == PROCESS_EVENT_EXIT)
        {
            // eger iþlem tamamlandý ya da bir þekilde bitirildi ise listeden çýkarýlacak.
            exit_process(p, p);
        }
        else
        {
            // iþlem devam edecek yani tekrar çaðýrýlmasý gerekiyorsa listedeki durumu koþan olarak deðiþtiriliyor.
            p->state = PROCESS_STATE_RUNNING;
        }
    }
}
/*---------------------------------------------------------------------------*/
void process_exit(struct process *p) // struct adresi girilmiþ olan iþlemi bitiren fonk.
{
    exit_process(p, PROCESS_CURRENT()); // eðer zaten kendi içerisinde ise exit iþlemlerini yapmasý için tekrar çaðýrýlmaz!
}
/*---------------------------------------------------------------------------*/
void process_init(void) // sistem kurulum fonk.
{
    // kaç adet evet barýndýrabileceði bilgisi
    lastevent = PROCESS_EVENT_MAX; // 0x8A
    // event sayýsý temizlenir.
    nevents = fevent = 0;
#if PROCESS_CONF_STATS
    process_maxevents = 0;
#endif /* PROCESS_CONF_STATS */
    // çalýþan iþlem ve iþlemler listesi temizlenir
    process_current = process_list = NULL;
}
/*---------------------------------------------------------------------------*/
static void do_poll(void)
{
    struct process *p;// temp bir p iþlemi oluþturulur
    // poll sorgulamasý kapatýlýr.
    poll_requested = 0;
    // poll komutuyla çalýþtýrýlmasý gereken tüm iþlemler çalýþtýrýlýr.
    for(p = process_list; p != NULL; p = p->next)
    {
        if(p->needspoll)
        {
            p->state = PROCESS_STATE_RUNNING;
            // iþlemin poll komutu silinir
            p->needspoll = 0;
            // o iþlem poll eventi ile bir kez çalýþtýrýlýr.
            call_process(p, PROCESS_EVENT_POLL, NULL);
        }
    }
}
/*---------------------------------------------------------------------------*/
static void do_event(void) // olay kuyruðunun çalýþtýrýldýðý fonk.
{
    process_event_t ev; // temp deðiþkenler tanýmlanýyor
    process_data_t data;
    struct process *receiver;
    struct process *p;

    // olaylarýn hepsinin iþletildiði yer burasýdýr.

    if(nevents > 0)   // eger kuyrukta bekleyen bir olay varsa girilir.
    {

        //son yapýlmýþ olaydan baþlayarak sýradaki yapýlmamýþ olayý temp register a al.
        // olay listesi ring buffer olarak kullanýlmýþ.
        ev = events[fevent].ev;
        data = events[fevent].data;
        receiver = events[fevent].p;

        //  yapýlmayý bekleyen olay sýrasý ring buffer dýþýna taþamayacaðý için mod almýþ ve sondan baþa döner
        fevent = (fevent + 1) % PROCESS_CONF_NUMEVENTS;
        // yapýlmayý bekleyenler sayýsý eksiltilir.
        --nevents;

        // eðer broadcast olayý gönderildiyse tüm iþlemler sýrasýyla çalýþtýrýlýr.
        if(receiver == PROCESS_BROADCAST)
        {
            for(p = process_list; p != NULL; p = p->next)
            {
                if(poll_requested)
                {
                    do_poll();
                }
                call_process(p, ev, data);
            }
        }
        else
        {
            // eger bir broadcast olayý deðilse sisteme özel bir olay girilmiþ ise burada yapýlýr.
            if(ev == PROCESS_EVENT_INIT)
            {
                // sisteme bir iþlemin init olayý gönderilmiþ ise o iþlemin durumu normal koþma moduna alýnýr.
                // tek seferlik yapýlacak iþ olduðundan dolayýdýr.
                receiver->state = PROCESS_STATE_RUNNING;
            }

            //diðer olaylar iþlemlere gönderilir.
            // bu sayede iki iþlem birbirine olay ve bildiri aktarabilir.
            // örnek;
            //PROCESS_EVENT_CONTINUE
            //PROCESS_EVENT_MSG
            call_process(receiver, ev, data);
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_run(void)
{
    // herhangi bir poll döngüsü iþi bekleniyor sa girilir.
    if(poll_requested)
    {
        // iþlemler poll olayý ile caliþtirilir
        do_poll();
    }

    // iþlemler broadcast olarak çalýþtýrýlýr.
    do_event();

    return nevents + poll_requested; // çalýþan iþlem sayýsý ve poll varsa +1 olarak sayýsý geri döndürülür
}
/*---------------------------------------------------------------------------*/
int process_nevents(void) // iþlem sayýsý sorgulama fonk.
{
    return nevents + poll_requested; // çalýþan iþlem sayýsý ve poll varsa +1 olarak sayýsý geri döndürülür
}
/*---------------------------------------------------------------------------*/
int process_post(struct process *p, process_event_t ev, process_data_t data) // olay gönderim fonk.
{
    process_num_events_t snum; // temp bir snum olusturulur

    if(PROCESS_CURRENT() == NULL)   // iþlemi nereden baþlatýldýðý bilgisi gönderilir.
    {
        PRINTF("process_post: NULL process posts event %d to process '%s', nevents %d\n",
               ev,PROCESS_NAME_STRING(p), nevents);
    }
    else
    {
        // eger iþlem baþka bir iþlem tarafýndan baþlatýlmýþ ise bu durum bildirilir.
        PRINTF("process_post: Process '%s' posts event %d to process '%s', nevents %d\n",
               PROCESS_NAME_STRING(PROCESS_CURRENT()), ev,
               p == PROCESS_BROADCAST? "<broadcast>": PROCESS_NAME_STRING(p), nevents);
    }
    //  eger sýradaki olay kuyruðunu max sayýya ulaþtýrdýysa ekrana bilgi verip çýkýlýr.
    // yapýlacak olay iþlemlerinde sayý sýnýrlamasý vardýr.
    if(nevents == PROCESS_CONF_NUMEVENTS)
    {
#if DEBUG
        if(p == PROCESS_BROADCAST)
        {
            printf("soft panic: event queue is full when broadcast event %d was posted from %s\n", ev, PROCESS_NAME_STRING(process_current));
        }
        else
        {
            printf("soft panic: event queue is full when event %d was posted to %s from %s\n", ev, PROCESS_NAME_STRING(p), PROCESS_NAME_STRING(process_current));
        }
#endif /* DEBUG */
        return PROCESS_ERR_FULL;
    }
    // iþlem olay kuyruðuna giriliyor
    snum = (process_num_events_t)(fevent + nevents) % PROCESS_CONF_NUMEVENTS;
    events[snum].ev = ev;
    events[snum].data = data;
    events[snum].p = p;
    // liste arttýrýlýyor.
    ++nevents;

#if PROCESS_CONF_STATS
    // max olay sýnýrý yedeklenir debug ve bilgi almak amaçlý. Yazýlýmýn en fazla olay kuyruðu oluþturduðu    gözlenebilir.
    if(nevents > process_maxevents)
    {
        process_maxevents = nevents;
    }
#endif /* PROCESS_CONF_STATS */

    return PROCESS_ERR_OK;
}
/*---------------------------------------------------------------------------*/
void process_post_synch(struct process *p, process_event_t ev, process_data_t data)
{
    struct process *caller = process_current; // temp bir structtanýmlanýyor
// yeni eklenen iþlem çalýþtýrýlýyor yada özel olarak tetikleniyor
    call_process(p, ev, data);
    // istenen iþlem çalýþtýrýldý artýk sistem listesinde kaldýðý iþten devam edebilir.
    process_current = caller;
}
/*---------------------------------------------------------------------------*/
void process_poll(struct process *p) // girilen iþlemi poll çalýþtýrma fonk.önden çalýþtýrma modu gibi kullanýlýr.
{
    if(p != NULL)   // girilen iþlem boþ deðiþse girer.
    {
        if(p->state == PROCESS_STATE_RUNNING ||
                p->state == PROCESS_STATE_CALLED)
        {
            p->needspoll = 1; // iþleme poll komutu gir.
            poll_requested = 1;// poll sorgusunu aç
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_is_running(struct process *p) // struct ile gönderilen iþlem çalýþýr durumda mý kontrol ediliyor.
{
    return p->state != PROCESS_STATE_NONE;
}
/*---------------------------------------------------------------------------*/

