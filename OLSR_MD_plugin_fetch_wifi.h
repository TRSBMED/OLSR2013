/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifdef OS_WIFI /* Optional - not supported on all platforms */

#ifndef LQ_ETX_FETCH_WIFI_
#define LQ_ETX_FETCH_WIFI_

#include "olsr_md_types.h"

#ifdef OS_WIFI
#include <net/ethernet.h>
#include "wifi_lnk_inf.h"
#endif

#define LQ_ALGORITHM_ETX_FETCH_WIFI_NAME "etx_fetch_wifi"

#define LQ_FETCH_WINDOW 32
#define LQ_FETCH_QUICKSTART_INIT 4

struct lq_fetch {
  uint8_t valLq;
  uint8_t valNlq;
#ifdef OS_WIFI
  uint8_t valBand;
  uint8_t valRSSI;
#endif
};

struct lq_fetch_hello {
  struct lq_fetch smoothed_lq;
  struct lq_fetch lq;
  uint8_t windowSize, activePtr;
  uint16_t last_seq_nr;
  uint16_t missed_hellos;
  bool perfect_eth;
  uint16_t received[LQ_FETCH_WINDOW], total[LQ_FETCH_WINDOW];
};

extern struct lq_hdlr lq_etx_fetch_wifi_hdlr;

#endif /* LQ_ETX_FETCH_WIFI_ */

#endif /* OS_WIFI */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
