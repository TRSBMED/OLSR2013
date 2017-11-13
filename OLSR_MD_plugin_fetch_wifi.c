/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifdef OS_WIFI /* Optional - not supported on all platforms */

#include "OLSR_MD_plugin_fetch_wifi.h"
#include "olsr_md_spf.h"
#include "lq_paquet.h"
#include "paquet.h"
#include "olsr_md.h"

#ifdef OS_WIFI
#include "wifi_lnk_info.h"
#define WEIGHT_ETX			50
#define WEIGHT_BAND	50
#endif

#define LQ_PLUG_LC_MULTIPLIER 1024
#define LQ_PLUG_RELEVANT_COSTCHANGE_FF 16

static void lq_init_fetch_wifi(void);

static olsr_md_lnkcost lq_calc_cost_fetch_wifi(const void *lq);

static void lq_paquet_loss_worker_fetch_wifi(struct lnk_entry *lnk, void *lq, bool lost);
static void lq_mem_foreign_hello_fetch_wifi(void *local, void *foreign);

static int lq_serial_hello_lq_pair_fetch_wifi(unsigned char *buff, void *lq);
static void lq_deserial_hello_lq_pair_fetch_wifi(const uint8_t ** curr, void *lq);
static int lq_serial_tc_lq_pair_fetch_wifi(unsigned char *buff, void *lq);
static void lq_deserial_tc_lq_pair_fetch_wifi(const uint8_t ** curr, void *lq);

static void lq_copy_lnk2neigh_fetch_wifi(void *t, void *s);
static void lq_copy_lnk2tc_fetch_wifi(void *target, void *source);
static void lq_clear_fetch_wifi(void *target);
static void lq_clear_fetch_wifi_hello(void *target);

static const char *lq_print_fetch_wifi(void *ptr, char separator, struct lqtextbuffer *buffer);
static const char *lq_print_cost_fetch_wifi(olsr_md_lnkcost cost, struct lqtextbuffer *buffer);

/* etx lq plugin (freifunk fpm version) settings */
struct lq_hdlr lq_etx_fetch_wifi_hdlr = {
  &lq_init_fetch_wifi,
  &lq_calc_cost_fetch_wifi,
  &lq_calc_cost_fetch_wifi,

  &lq_paquet_loss_worker_fetch_wifi,

  &lq_mem_foreign_hello_fetch_wifi,
  &lq_copy_lnk2neigh_fetch_wifi,
  &lq_copy_lnk2tc_fetch_wifi,
  &lq_clear_fetch_wifi_hello,
  &lq_clear_fetch_wifi,

  &lq_serial_hello_lq_pair_fetch_wifi,
  &lq_serial_tc_lq_pair_fetch_wifi,
  &lq_deserial_hello_lq_pair_fetch_wifi,
  &lq_deserial_tc_lq_pair_fetch_wifi,

  &lq_print_fetch_wifi,
  &lq_print_fetch_wifi,
  &lq_print_cost_fetch_wifi,

  sizeof(struct lq_fetch_hello),
  sizeof(struct lq_fetch),
  4,
  4
};

static void
lq_fetch_wifi_hdl_lqchange(void) {
  struct lq_fetch_hello *lq;
  struct ipaddress_str buf;
  struct lnk_entry *lnk;

  bool triggered = false;

  OLSR_MD_FOR_ALL_LNK_ENTRIES(lnk) {
    bool relevant = false;
    lq = (struct lq_fetch_hello *)lnk->lnkqual;

#if 0
  fprintf(stderr, "%s: old = %u/%u   new = %u/%u\n", olsr_md_ip_to_string(&buf, &lnk->neighbor_iface_address),
      lq->smoothed_lq.valLq, lq->smoothed_lq.valNlq,
      lq->lq.valLq, lq->lq.valNlq);
#endif

    if (lq->smoothed_lq.valLq < lq->lq.valLq) {
      if (lq->lq.valLq >= 254 || lq->lq.valLq - lq->smoothed_lq.valLq > lq->smoothed_lq.valLq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valLq > lq->lq.valLq) {
      if (lq->smoothed_lq.valLq - lq->lq.valLq > lq->smoothed_lq.valLq/10) {
        relevant = true;
      }
    }
    if (lq->smoothed_lq.valNlq < lq->lq.valNlq) {
      if (lq->lq.valNlq >= 254 || lq->lq.valNlq - lq->smoothed_lq.valNlq > lq->smoothed_lq.valNlq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valNlq > lq->lq.valNlq) {
      if (lq->smoothed_lq.valNlq - lq->lq.valNlq > lq->smoothed_lq.valNlq/10) {
        relevant = true;
      }
    }

    if (relevant) {
      memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct lq_fetch));
      lnk->lnkcost = lq_calc_cost_fetch_wifi(&lq->smoothed_lq);
      triggered = true;
    }
  } OLSR_MD_FOR_ALL_LNK_ENTRIES_END(lnk)

  if (!triggered) {
    return;
  }

  OLSR_MD_FOR_ALL_LNK_ENTRIES(lnk) {
    lq = (struct lq_fetch_hello *)lnk->lnkqual;

    if (lq->smoothed_lq.valLq >= 254 && lq->smoothed_lq.valNlq >= 254) {
      continue;
    }

    if (lq->smoothed_lq.valLq == lq->lq.valLq && lq->smoothed_lq.valNlq == lq->lq.valNlq) {
      continue;
    }

    memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct lq_fetch));
    lnk->lnkcost = lq_calc_cost_fetch_wifi(&lq->smoothed_lq);
  } OLSR_MD_FOR_ALL_LNK_ENTRIES_END(lnk)

  olsr_md_relevant_lnkcost_change();
}

static void
lq_parser_fetch_wifi(struct olsr_md *olsr_md, struct iface *in_if, union olsr_md_ip_address *from_address)
{
  const union olsr_md_ip_address *main_address;
  struct lnk_entry *lnk;
  struct lq_fetch_hello *lq;
  uint32_t seq_diff;

  /* Find main addressess */
  main_address = mid_lookup_main_address(from_address);

  /* Loopup lnk entry */
  lnk = lookup_lnk_entry(from_address, main_address, in_if);
  if (lnk == NULL) {
    return;
  }

  lq = (struct lq_fetch_hello *)lnk->lnkqual;

  /* ignore double package */
  if (lq->last_seq_nr == olsr_md->olsr_md_seqno) {
    struct ipaddress_str buf;
    olsr_md_syslog(OLSR_MD_LOG_INF, "detected duplicate paquet with seqnr %d from %s on %s (%d Bytes)",
		olsr_md->olsr_md_seqno,olsr_md_ip_to_string(&buf, from_address),in_if->int_name,ntohs(olsr_md->olsr_md_packlen));
    return;
  }

  if (lq->last_seq_nr > olsr_md->olsr_md_seqno) {
    seq_diff = (uint32_t) olsr_md->olsr_md_seqno + 65536 - lq->last_seq_nr;
  } else {
    seq_diff = olsr_md->olsr_md_seqno - lq->last_seq_nr;
  }

  /* Jump in sequence numbers ? */
  if (seq_diff > 256) {
    seq_diff = 1;
  }

  lq->received[lq->activePtr]++;
  lq->total[lq->activePtr] += seq_diff;

  lq->last_seq_nr = olsr_md->olsr_md_seqno;
  lq->missed_hellos = 0;
}

static void
lq_fetch_wifi_timer(void __attribute__ ((unused)) * context)
{
  struct lnk_entry *lnk;

#ifdef OS_WIFI
	wifi_lnk_inf_get();
#endif

  OLSR_MD_FOR_ALL_LNK_ENTRIES(lnk) {
    struct lq_fetch_hello *tlq = (struct lq_fetch_hello *)lnk->lnkqual;
    fpm ratio;
    int i, received, total;

    received = 0;
    total = 0;

    /* enetlnkarge window if still in quickstart phase */
    if (tlq->windowSize < LQ_FETCH_WINDOW) {
      tlq->windowSize++;
    }
    for (i = 0; i < tlq->windowSize; i++) {
      received += tlq->received[i];
      total += tlq->total[i];
    }

    /* calculate lnk qual */
    if (total == 0) {
      tlq->lq.valLq = 0;
    } else {
      // start with lnk-loss-factor
      ratio = fpmidiv(itofpm(lnk->loss_lnk_multiplier), LNK_LOSS_MULTIPLIER);

      /* keep missed hello periods in mind (round up hello interval to seconds) */
      if (tlq->missed_hellos > 1) {
        received = received - received * tlq->missed_hellos * lnk->inter->hello_etime/1000 / LQ_FETCH_WINDOW;
      }

      // calculate received/total factor
      ratio = fpmmuli(ratio, received);
      ratio = fpmidiv(ratio, total);
      ratio = fpmmuli(ratio, 255);

      tlq->lq.valLq = (uint8_t) (fpmtoi(ratio));
    }

    /* ethernet booster */
    if (lnk->inter->mode == IF_MODE_ETHERNET) {
      if (tlq->lq.valLq > (uint8_t)(0.95 * 255)) {
        tlq->perfect_eth = true;
      }
      else if (tlq->lq.valLq > (uint8_t)(0.90 * 255)) {
        tlq->perfect_eth = false;
      }

      if (tlq->perfect_eth) {
        tlq->lq.valLq = 255;
      }
    }
    else if (lnk->inter->mode != IF_MODE_ETHERNET && tlq->lq.valLq > 0) {
      tlq->lq.valLq--;
    }

    // shift buffer
    tlq->activePtr = (tlq->activePtr + 1) % LQ_FETCH_WINDOW;
    tlq->total[tlq->activePtr] = 0;
    tlq->received[tlq->activePtr] = 0;

  } OLSR_MD_FOR_ALL_LNK_ENTRIES_END(lnk);

  lq_fetch_wifi_hdl_lqchange();
}

static void
lq_init_fetch_wifi(void)
{
  if (olsr_md_cnf->lq_nat_thresh < 1.0f) {
    fprintf(stderr, "Warning, nat_treshold < 1.0 is more likely to produce loops with etx_fetch\n");
  }
  olsr_md_paquetparser_add_function(&lq_parser_fetch_wifi);
  olsr_md_start_timer(1000, 0, OLSR_MD_TIMER_PERIODIC, &lq_fetch_wifi_timer, NULL, 0);

#ifdef OS_WIFI
  wifi_lnk_inf_init();
#endif
}

static olsr_md_lnkcost
lq_calc_cost_fetch_wifi(const void *ptr)
{
  const struct lq_fetch *lq = ptr;
  olsr_md_lnkcost cost;
  bool ether;
  int lq_int, netlnkq_int;
#ifdef OS_WIFI
  fpm wifi = itofpm((int) lq->valBand + lq->valRSSI);
#endif

  // MIN_USEFUL_LQ is a float, multiplying by 255 converts it to uint8_t
  if (lq->valLq < (unsigned int)(255 * MIN_USEFUL_LQ) || lq->valNlq < (unsigned int)(255 * MIN_USEFUL_LQ)) {
    return LNK_COST_BROKEN;
  }

  ether = lq->valLq == 255 && lq->valNlq == 255;

  lq_int = (int)lq->valLq;
  if (lq_int > 0 && lq_int < 255) {
    lq_int++;
  }

  netlnkq_int = (int)lq->valNlq;
  if (netlnkq_int > 0 && netlnkq_int < 255) {
    netlnkq_int++;
  }

#ifdef OS_WIFI
  wifi = fpmidiv(wifi, 255);
  cost = fpmidiv(itofpm(255 * 255), lq_int * netlnkq_int); // 1 / (LQ * NLQ)
  cost = fpmadd(cost, wifi);
#else
  cost = fpmidiv(itofpm(255 * 255), lq_int * netlnkq_int); // 1 / (LQ * NLQ)
#endif
  if (ether) {
    /* ethernet boost */
    cost /= 10;
  }

  if (cost > LNK_COST_BROKEN)
    return LNK_COST_BROKEN;
  if (cost == 0)
    return 1;
  return cost;
}

static int
lq_serial_hello_lq_pair_fetch_wifi(unsigned char *buff, void *ptr)
{
  struct lq_fetch *lq = ptr;

#ifdef OS_WIFI
  buff[0] = lq->valBand;
  buff[1] = lq->valRSSI;
#else
  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
#endif
  buff[2] = (unsigned char)lq->valLq;
  buff[3] = (unsigned char)lq->valNlq;

  return 4;
}

static void
lq_deserial_hello_lq_pair_fetch_wifi(const uint8_t ** curr, void *ptr)
{
  struct lq_fetch *lq = ptr;

#ifdef OS_WIFI
  pkt_get_u8(curr, &lq->valBand);
  pkt_get_u8(curr, &lq->valRSSI);
#else
  pkt_ignore_u16(curr);
#endif
  pkt_get_u8(curr, &lq->valLq);
  pkt_get_u8(curr, &lq->valNlq);
}

static int
lq_serial_tc_lq_pair_fetch_wifi(unsigned char *buff, void *ptr)
{
  struct lq_fetch *lq = ptr;

#ifdef OS_WIFI
  buff[0] = lq->valBand;
  buff[1] = lq->valRSSI;
#else
  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
#endif
  buff[2] = (unsigned char)lq->valLq;
  buff[3] = (unsigned char)lq->valNlq;

  return 4;
}

static void
lq_deserial_tc_lq_pair_fetch_wifi(const uint8_t ** curr, void *ptr)
{
  struct lq_fetch *lq = ptr;

#ifdef OS_WIFI
  pkt_get_u8(curr, &lq->valBand);
  pkt_get_u8(curr, &lq->valRSSI);
#else
  pkt_ignore_u16(curr);
#endif
  pkt_get_u8(curr, &lq->valLq);
  pkt_get_u8(curr, &lq->valNlq);
}

static void
lq_paquet_loss_worker_fetch_wifi(struct lnk_entry *lnk,
    void __attribute__ ((unused)) *ptr, bool lost)
{
  struct lq_fetch_hello *tlq = (struct lq_fetch_hello *)lnk->lnkqual;

  if (lost) {
    tlq->missed_hellos++;
  }
  return;
}

static void
lq_mem_foreign_hello_fetch_wifi(void *ptrLocal, void *ptrForeign)
{
  struct lq_fetch_hello *local = ptrLocal;
  struct lq_fetch *foreign = ptrForeign;

  if (foreign) {
    local->lq.valNlq = foreign->valLq;
  } else {
    local->lq.valNlq = 0;
  }
}

static void
lq_copy_lnk2neigh_fetch_wifi(void *t, void *s)
{
  struct lq_fetch *target = t;
  struct lq_fetch_hello *source = s;
  *target = source->smoothed_lq;
}

static void
lq_copy_lnk2tc_fetch_wifi(void *t, void *s)
{
  struct lq_fetch *target = t;
  struct lq_fetch_hello *source = s;
  *target = source->smoothed_lq;
}

static void
lq_clear_fetch_wifi(void *target)
{
  memset(target, 0, sizeof(struct lq_fetch));
}

static void
lq_clear_fetch_wifi_hello(void *target)
{
  struct lq_fetch_hello *local = target;
  int i;

  lq_clear_fetch_wifi(&local->lq);
  lq_clear_fetch_wifi(&local->smoothed_lq);
  local->windowSize = LQ_FETCH_QUICKSTART_INIT;
  for (i = 0; i < LQ_FETCH_WINDOW; i++) {
    local->total[i] = 3;
  }
}

static const char *
lq_print_fetch_wifi(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct lq_fetch *lq = ptr;
  int lq_int, netlnkq_int;

  lq_int = (int)lq->valLq;
  if (lq_int > 0 && lq_int < 255) {
    lq_int++;
  }

  netlnkq_int = (int)lq->valNlq;
  if (netlnkq_int > 0 && netlnkq_int < 255) {
    netlnkq_int++;
  }

  snprintf(buffer->buf, sizeof(buffer->buf), "%s%c%s", fpmtoa(fpmidiv(itofpm(lq_int), 255)), separator,
           fpmtoa(fpmidiv(itofpm(netlnkq_int), 255)));
  return buffer->buf;
}

static const char *
lq_print_cost_fetch_wifi(olsr_md_lnkcost cost, struct lqtextbuffer *buffer)
{
  snprintf(buffer->buf, sizeof(buffer->buf), "%s", fpmtoa(cost));
  return buffer->buf;
}

#endif /* OS_WIFI */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
