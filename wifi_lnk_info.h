/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifdef OS_WIFI /* Optional - not supported on all platforms */

#ifndef OLSR_MD_OS_WIFI_LNK_INF_H_INCLUDED
#define OLSR_MD_OS_WIFI_LNK_INF_H_INCLUDED

#include <net/ethernet.h>
#include "lnk_set.h"

struct lq_wifi_dat {
	unsigned char mac[ETHERNET_ADDRESS_LEN]; // MAC addressess of station
	int8_t sig; // Signal level in dBm
	uint16_t band; // Active band setting in 100kbit/sec
	struct lq_wifi_dat *next; // Linked list pointer
};

void wifi_lnk_inf_init(void);
void wifi_lnk_inf_cleanup(void);
void wifi_lnk_inf_get(void);

#endif // OLSR_MD_OS_WIFI_LNK_INF_H_INCLUDED

#endif /* OS_WIFI */
