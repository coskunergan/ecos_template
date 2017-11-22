/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */
#include "ecos.h"
#include "process.h"

// pointer structure lar�
struct process *process_list = NULL; // i�lem listesi seft referance struct �eklinde birbirine ba�lan�rlar.
struct process *process_current = NULL; // �al��makta olan i�lemin tutuldu�u struct

static process_event_t lastevent; // olay say�s�n� tutan tan�mlama

struct event_data   // olay listesi parametereleri
{
    process_event_t ev; // olay�n tan�m�
    process_data_t data;// olay�n datas� e�er olay mesaj g�ndermek ise i�lemler birbirine data g�nderebilir.
    struct process *p; // olay� ger�ekle�tirecek olan i�lem
};

// bu iki register olay listesini ring buffer mant���nda kullan�lmas�n� sa�lar
static process_num_events_t nevents; // olay listesine olay eklenmesi art���n� tutacak adres max kontrol� i�in
static process_num_events_t fevent;  // olay listesinde i�lenen olay indexinin tutacak adres
// olay dizisinin struct adresinin tan�mlanmas�
static struct event_data events[PROCESS_CONF_NUMEVENTS];

#if PROCESS_CONF_STATS // eger ula��lm�� max olay say�s�n�n tutulmas� isteniyorsa tan�mlanacak
process_num_events_t process_maxevents; // max olay sayac� tan�mlamas�
#endif

static volatile unsigned char poll_requested; // poll �al��t�r�lan fonksiyon varsa aktif edilen flag

#define PROCESS_STATE_NONE        0 // i�lemin silinmesi ve ya bo� olmas� durumu
#define PROCESS_STATE_RUNNING     1 // i�lemin hala listede ko�acak durum da bekledi�i durum
#define PROCESS_STATE_CALLED      2 // i�lemin �al���r halde oladu�u durum

static void call_process(struct process *p, process_event_t ev, process_data_t data);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
process_event_t process_alloc_event(void)  //  her �a��r�ld�g�nda olay say�s� artar
{
    return lastevent++;
}
/*---------------------------------------------------------------------------*/
void process_start(struct process *p, process_data_t data) // i�lem bas�latan fonk.
{
    struct process *q; // temp bir struct a��l�r

    // �al��t�r�lmak istenen i�lem, listede zaten �al���yor mu diye kontrol ediliryor
    // listenin sonunda NULL olaca�� i�in i�lem listesi bitince d�ng�den ��kar
    for(q = process_list; q != p && q != NULL; q = q->next);

    // e�er i�lem zaten �al���yorsa ��kar
    if(q == p)
    {
        return;
    }
    // p->next art�k listenin sonunu i�aret ediyor ve Null dur listenin en ba��ndaki i�lem en sonuna ba�lan�r.
    p->next = process_list;
    // bo�a ��kan liste ba�� i�lemi yerine yeni �al��t�r�lacak i�lem y�klenir.
    process_list = p;
    // durumu ko�ar olarak de�i�tirilir.
    p->state = PROCESS_STATE_RUNNING;
    // PT nin switch case de�eri s�f�rlan�r.
    PT_INIT(&p->pt);

    // debug comdan bilgi yazd�r�l�r.
    PRINTF("process: starting '%s'\n", PROCESS_NAME_STRING(p));

    // art�k i�lem listeye eklendi �al��t�r�lmas� i�in post edilmesi gerekiyor.
    // i�lemin eventine ilk defa �al��t��� i�in kurulacak bilgisi giriliyor.
    // i�lemin datas� aynen aktar�l�yor.
    process_post_synch(p, PROCESS_EVENT_INIT, data);
}
/*---------------------------------------------------------------------------*/
static void exit_process(struct process *p, struct process *fromprocess)
{
    register struct process *q; // temp struct olu�turulur.
    struct process *old_current = process_current;  // �uanda �al���r durumda olan ba�ka bir i�lem varsa
    // bitirilecek i�lem ile ilgili �al��malar yap�lana kadar yedekleniyor

    // ekrana bilgi g�nderilir
    PRINTF("process: exit_process '%s'\n", PROCESS_NAME_STRING(p));

    /* i�lem listesi kontrol edilip bitirilecek i�lem listede olup olmad���na bak�l�yor */
    for(q = process_list; q != p && q != NULL; q = q->next);
    if(q == NULL)
    {
        return; //bulunamad� ise ��k�l�yor. (listede �al���r olmayan i�lem bitirilemez)
    }

    if(process_is_running(p))   // eger bitirilece ki�lemin durumu ko�ar yada �a�r�lm�� ise girilir.
    {
        // i�lemin durumu yok olarak i�aretlenir
        p->state = PROCESS_STATE_NONE;

        // bitirilecek i�lem listedeki b�t�n i�lemle �a��r�larak HEPS�NE bildiriliyor.
        for(q = process_list; q != NULL; q = q->next)
        {
            if(p != q)
            {
                // di�er i�lemlere bitirilen i�lemin verisi de data k�sm�ndan g�nderiliyor.
                // bu sayede birbirine ba�l� olarak �al��an i�lerde kendilerini kapatabilir.
                call_process(q, PROCESS_EVENT_EXITED, (process_data_t)p);
            }
        }
        // eger i�lem kendisi de�ilde ba�ka bir i�lem taraf�ndan kapat�l�yorsa girilir.
        if(p->thread != NULL && p != fromprocess)
        {
            // burada i�lem kapan�� i�lerini yapmas� i�in son bir kez �a��r�l�r.
            process_current = p;
            p->thread(&p->pt, PROCESS_EVENT_EXIT, NULL);
        }
    }
    // i�lem listenin ilk �yesimiydi.
    if(p == process_list)
    {
        // ilk �ye ezilerek yerine bir sonraki �ye eklenir.
        process_list = process_list->next;
    }
    else
    {
        // i�lem listenin neresinde bulunuyor
        for(q = process_list; q != NULL; q = q->next)
        {
            if(q->next == p)
            {
                // i�lem bulundu adresi ezilip yerine bir sonraki i�lem eklenerek bitiriliyor.
                q->next = p->next;
                break;
            }
        }
    }
    // daha �nce �al���r durumda olan herhangi bir i�lem kald�g� yerden devam ettiriliyor
    process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void call_process(struct process *p, process_event_t ev, process_data_t data)
{
    int ret; // geri d�n�� degi�keni tan�mlan�r

#if DEBUG // debug com dan her i�lem �a��r�ld���nda bilgi yazd�r�labilir ve event sayesinde o i�lemin ad� ve a�amas� ��renilebilir.
    if(p->state == PROCESS_STATE_CALLED)
    {
        printf("process: process '%s' called again with event %d\n", PROCESS_NAME_STRING(p), ev);
    }
#endif /* DEBUG */
    // �al��t�r�lacak i�lem ko�uyorsa ve fonk. adresi null de�ilse devam edilir.
    if((p->state & PROCESS_STATE_RUNNING) &&
            p->thread != NULL)
    {
        // ekrana bilgilendirme yaz�l�r.
        PRINTF("process: calling process '%s' with event %d\n", PROCESS_NAME_STRING(p), ev);
        // �al��an i�leme adresi y�klenir.
        process_current = p;
        // durumu �a�r�ld� olarak dei�tirilir.
        p->state = PROCESS_STATE_CALLED;
        // fonksiyon pointer dan protothreads li yap�da olu�turulmu� fonksiyon adresine dallan�l�r.
        ret = p->thread(&p->pt, ev, data);
        // i�lem �al��t�r�ld� ve geri d�n�ld� geri d�n�� bilgisine bak�l�yor
        if(ret == PT_EXITED ||
                ret == PT_ENDED ||
                ev == PROCESS_EVENT_EXIT)
        {
            // eger i�lem tamamland� ya da bir �ekilde bitirildi ise listeden ��kar�lacak.
            exit_process(p, p);
        }
        else
        {
            // i�lem devam edecek yani tekrar �a��r�lmas� gerekiyorsa listedeki durumu ko�an olarak de�i�tiriliyor.
            p->state = PROCESS_STATE_RUNNING;
        }
    }
}
/*---------------------------------------------------------------------------*/
void process_exit(struct process *p) // struct adresi girilmi� olan i�lemi bitiren fonk.
{
    exit_process(p, PROCESS_CURRENT()); // e�er zaten kendi i�erisinde ise exit i�lemlerini yapmas� i�in tekrar �a��r�lmaz!
}
/*---------------------------------------------------------------------------*/
void process_init(void) // sistem kurulum fonk.
{
    // ka� adet evet bar�nd�rabilece�i bilgisi
    lastevent = PROCESS_EVENT_MAX; // 0x8A
    // event say�s� temizlenir.
    nevents = fevent = 0;
#if PROCESS_CONF_STATS
    process_maxevents = 0;
#endif /* PROCESS_CONF_STATS */
    // �al��an i�lem ve i�lemler listesi temizlenir
    process_current = process_list = NULL;
}
/*---------------------------------------------------------------------------*/
static void do_poll(void)
{
    struct process *p;// temp bir p i�lemi olu�turulur
    // poll sorgulamas� kapat�l�r.
    poll_requested = 0;
    // poll komutuyla �al��t�r�lmas� gereken t�m i�lemler �al��t�r�l�r.
    for(p = process_list; p != NULL; p = p->next)
    {
        if(p->needspoll)
        {
            p->state = PROCESS_STATE_RUNNING;
            // i�lemin poll komutu silinir
            p->needspoll = 0;
            // o i�lem poll eventi ile bir kez �al��t�r�l�r.
            call_process(p, PROCESS_EVENT_POLL, NULL);
        }
    }
}
/*---------------------------------------------------------------------------*/
static void do_event(void) // olay kuyru�unun �al��t�r�ld��� fonk.
{
    process_event_t ev; // temp de�i�kenler tan�mlan�yor
    process_data_t data;
    struct process *receiver;
    struct process *p;

    // olaylar�n hepsinin i�letildi�i yer buras�d�r.

    if(nevents > 0)   // eger kuyrukta bekleyen bir olay varsa girilir.
    {

        //son yap�lm�� olaydan ba�layarak s�radaki yap�lmam�� olay� temp register a al.
        // olay listesi ring buffer olarak kullan�lm��.
        ev = events[fevent].ev;
        data = events[fevent].data;
        receiver = events[fevent].p;

        //  yap�lmay� bekleyen olay s�ras� ring buffer d���na ta�amayaca�� i�in mod alm�� ve sondan ba�a d�ner
        fevent = (fevent + 1) % PROCESS_CONF_NUMEVENTS;
        // yap�lmay� bekleyenler say�s� eksiltilir.
        --nevents;

        // e�er broadcast olay� g�nderildiyse t�m i�lemler s�ras�yla �al��t�r�l�r.
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
            // eger bir broadcast olay� de�ilse sisteme �zel bir olay girilmi� ise burada yap�l�r.
            if(ev == PROCESS_EVENT_INIT)
            {
                // sisteme bir i�lemin init olay� g�nderilmi� ise o i�lemin durumu normal ko�ma moduna al�n�r.
                // tek seferlik yap�lacak i� oldu�undan dolay�d�r.
                receiver->state = PROCESS_STATE_RUNNING;
            }

            //di�er olaylar i�lemlere g�nderilir.
            // bu sayede iki i�lem birbirine olay ve bildiri aktarabilir.
            // �rnek;
            //PROCESS_EVENT_CONTINUE
            //PROCESS_EVENT_MSG
            call_process(receiver, ev, data);
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_run(void)
{
    // herhangi bir poll d�ng�s� i�i bekleniyor sa girilir.
    if(poll_requested)
    {
        // i�lemler poll olay� ile cali�tirilir
        do_poll();
    }

    // i�lemler broadcast olarak �al��t�r�l�r.
    do_event();

    return nevents + poll_requested; // �al��an i�lem say�s� ve poll varsa +1 olarak say�s� geri d�nd�r�l�r
}
/*---------------------------------------------------------------------------*/
int process_nevents(void) // i�lem say�s� sorgulama fonk.
{
    return nevents + poll_requested; // �al��an i�lem say�s� ve poll varsa +1 olarak say�s� geri d�nd�r�l�r
}
/*---------------------------------------------------------------------------*/
int process_post(struct process *p, process_event_t ev, process_data_t data) // olay g�nderim fonk.
{
    process_num_events_t snum; // temp bir snum olusturulur

    if(PROCESS_CURRENT() == NULL)   // i�lemi nereden ba�lat�ld��� bilgisi g�nderilir.
    {
        PRINTF("process_post: NULL process posts event %d to process '%s', nevents %d\n",
               ev,PROCESS_NAME_STRING(p), nevents);
    }
    else
    {
        // eger i�lem ba�ka bir i�lem taraf�ndan ba�lat�lm�� ise bu durum bildirilir.
        PRINTF("process_post: Process '%s' posts event %d to process '%s', nevents %d\n",
               PROCESS_NAME_STRING(PROCESS_CURRENT()), ev,
               p == PROCESS_BROADCAST? "<broadcast>": PROCESS_NAME_STRING(p), nevents);
    }
    //  eger s�radaki olay kuyru�unu max say�ya ula�t�rd�ysa ekrana bilgi verip ��k�l�r.
    // yap�lacak olay i�lemlerinde say� s�n�rlamas� vard�r.
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
    // i�lem olay kuyru�una giriliyor
    snum = (process_num_events_t)(fevent + nevents) % PROCESS_CONF_NUMEVENTS;
    events[snum].ev = ev;
    events[snum].data = data;
    events[snum].p = p;
    // liste artt�r�l�yor.
    ++nevents;

#if PROCESS_CONF_STATS
    // max olay s�n�r� yedeklenir debug ve bilgi almak ama�l�. Yaz�l�m�n en fazla olay kuyru�u olu�turdu�u    g�zlenebilir.
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
    struct process *caller = process_current; // temp bir structtan�mlan�yor
// yeni eklenen i�lem �al��t�r�l�yor yada �zel olarak tetikleniyor
    call_process(p, ev, data);
    // istenen i�lem �al��t�r�ld� art�k sistem listesinde kald��� i�ten devam edebilir.
    process_current = caller;
}
/*---------------------------------------------------------------------------*/
void process_poll(struct process *p) // girilen i�lemi poll �al��t�rma fonk.�nden �al��t�rma modu gibi kullan�l�r.
{
    if(p != NULL)   // girilen i�lem bo� de�i�se girer.
    {
        if(p->state == PROCESS_STATE_RUNNING ||
                p->state == PROCESS_STATE_CALLED)
        {
            p->needspoll = 1; // i�leme poll komutu gir.
            poll_requested = 1;// poll sorgusunu a�
        }
    }
}
/*---------------------------------------------------------------------------*/
int process_is_running(struct process *p) // struct ile g�nderilen i�lem �al���r durumda m� kontrol ediliyor.
{
    return p->state != PROCESS_STATE_NONE;
}
/*---------------------------------------------------------------------------*/

