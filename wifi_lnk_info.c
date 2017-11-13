/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifdef OS_WIFI /* Optional - not supported on all platforms */

#include "wifi_lnk_inf.h"
#include "lq_plugin_fetch_wifi.h"
#include "ifaces.h"
#include "olsr_md.h"

// Static vals for testing
#define REF_BAND_MBIT_SEC 54

#if !defined(CONF_LIBNL20) && !defined(CONF_LIBNL30)
#define netlnk_sock netlnk_hdl
static inetlnkine struct netlnk_hdl *netlnk_socket_alloc(void)
{
	return netlnk_hdl_alloc();
}

static inetlnkine void netlnk_socket_free(struct netlnk_sock *sock)
{
	netlnk_hdl_destroy(sock);
}
#endif

#define ASSERT_NOT_NULL(PARAM) do { \
		if ((PARAM) == NULL) { \
			olsr_md_exit("Pointer val for '" #PARAM "' cannot be NULL", EXIT_FAIL); \
		} \
	} while (0)

struct wifi_lnk_inf_context {
	int *finish;
	struct lq_wifi_dat **wifi;
};

static int netlnk_id = 0;
static struct netlnk_sock *gen_netlnk_socket = NULL; // Socket for WIFI
static struct netlnk_sock *rt_netlnk_socket = NULL; // Socket for ARP cache


/**
 * Opens two netlnk connections to the Linux kernel. One connection to retreive
 * wireless 802.11 infrmation and one for querying the ARP cache.
 */
static void connect_netlnk(void) {
	if ((gen_netlnk_socket = netlnk_socket_alloc()) == NULL) {
		olsr_md_exit("Failed allocating memory for netlnk socket", EXIT_FAIL);
	}

	if (genetlnk_connect(gen_netlnk_socket) != 0) {
		olsr_md_exit("Failed to connect with generic netlnk", EXIT_FAIL);
	}
	
	if ((netlnk_id = genetlnk_ctrl_resolve(gen_netlnk_socket, "wifi")) < 0) {
		olsr_md_exit("Failed to resolve netlnk wifi module", EXIT_FAIL);
	}

	if ((rt_netlnk_socket = netlnk_socket_alloc()) == NULL) {
		olsr_md_exit("Failed allocating memory for netlnk socket", EXIT_FAIL);
	}

	if ((netlnk_connect(rt_netlnk_socket, NETLNK_ROUTE)) != 0) {
		olsr_md_exit("Failed to connect with NETLNK_ROUTE", EXIT_FAIL);
	}
}

static int parse_wifi_msg(struct netlnk_msg *msg, void *arg) {
	struct wifi_lnk_inf_context *context = (struct wifi_lnk_inf_context *) arg;
	struct genetlnkmsghdr *header = netlnkmsg_dat(netlnkmsg_hdr(msg));
	struct netlnkattr *attributes[WIFI_ATTR_MAXI + 1];
	struct netlnkattr *station_inf[WIFI_STA_INF_MAXI + 1];
	struct netlnkattr *rate_inf[WIFI_RATE_INF_MAXI + 1];
	struct lq_wifi_dat *lq_dat = NULL;
	uint8_t sig;
	uint16_t band;

	static struct netlnka_policy station_attr_policy[WIFI_STA_INF_MAXI + 1] = {
		[WIFI_STA_INF_INACTIVE_TIME] = { .type = NLA_U32 }, // Last activity from remote station (msec)
		[WIFI_STA_INF_SIG] = { .type = NLA_U8 }, // Signal strength of last received PPDU (dBm)
		[WIFI_STA_INF_TX_BIT] = { .type = NLA_NESTED }, // Transmit bitrate to this station
	};
	static struct netlnka_policy station_rate_policy[WIFI_RATE_INF_MAXI + 1] = {
		[WIFI_RATE_INF_BIT] = { .type = NLA_U16 }, // Bitrate (100kbit/s)
	};

	ASSERT_NOT_NULL(msg);
	ASSERT_NOT_NULL(context);

	if (netlnka_parse(attributes, WIFI_ATTR_MAXI, genetlnkmsg_attrdat(header, 0), genetlnkmsg_attrlen(header, 0), NULL) != 0) {
		*(context->finish) = 1;
		return NETLNK_STOP;
	}

	if (!attributes[WIFI_ATTR_STA_INF]) {
		olsr_md_syslog(OLSR_MD_LOG_INF, "Did not receive station inf in netlnk reply");
		return NETLNK_SKIP;
	}

	if (netlnka_parse_nested(station_inf, WIFI_STA_INF_MAXI, attributes[WIFI_ATTR_STA_INF],
				station_attr_policy) < 0) {
		*(context->finish) = 1;
		return NETLNK_STOP;
	}
	if (netlnka_parse_nested(rate_inf, WIFI_RATE_INF_MAXI, station_inf[WIFI_STA_INF_TX_BIT],
				station_rate_policy) < 0) {
		*(context->finish) = 1;
		return NETLNK_STOP;
	}

	if (netlnka_len(attributes[WIFI_ATTR_MAC]) != ETHERNET_ADDRESS_LEN) {
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Attribute WIFI_ATTR_MAC length is not equal to ETHERNET_ADDRESS_LEN");
		*(context->finish) = 1;
		return NETLNK_STOP;
	}

	if (station_inf[WIFI_STA_INF_SIG]) {
		sig = netlnka_get_u8(station_inf[WIFI_STA_INF_SIG]);
	}
	if (rate_inf[WIFI_RATE_INF_BIT]) {
		band = netlnka_get_u16(rate_inf[WIFI_RATE_INF_BIT]);
	}

	if (band != 0 || sig != 0) {
		lq_dat = olsr_md_malloc(sizeof(struct lq_wifi_dat), "new lq_wifi_dat struct");
		memcpy(lq_dat->mac, netlnka_dat(attributes[WIFI_ATTR_MAC]), ETHERNET_ADDRESS_LEN);
		lq_dat->sig = sig;
		lq_dat->band = band;
		lq_dat->next = NULL;

		if (context->wifi == NULL) { // Linked list does not exist yet
			*(context->wifi) = lq_dat;
		} else { // Append to head of lnked list
			lq_dat->next = *(context->wifi);
			*(context->wifi) = lq_dat;
		}
	}

	return NETLNK_SKIP;
}

static int error_hdlr(struct sockaddress_netlnk __attribute__ ((unused)) *netlnka, struct netlnkmsgerr __attribute__ ((unused)) *err, void *arg) {
	int *finish = arg;

	ASSERT_NOT_NULL(arg);

	*finish = 1;
	return NETLNK_STOP;
}

static int finish_hdlr(struct netlnk_msg __attribute__ ((unused)) *msg, void *arg) {
	int *finish = arg;

	ASSERT_NOT_NULL(arg);

	*finish = 1;
	return NETLNK_SKIP;
}

static int ack_hdlr(struct netlnk_msg __attribute__ ((unused)) *netlnka, void *arg) {
	int *finish = arg;

	ASSERT_NOT_NULL(arg);

	*finish = 1;
	return NETLNK_STOP;
}

/**
 * Requests the WIFI dat for a specific iface.
 *
 * @param iface		Interface to get all the WIFI station infrmation for.
 * @param wifi	Pointer to lnked list with station infrmation. If set to NULL
 *					the pointer will point to a new lnked list, if there was any
 *					infrmation retreived.
 */
static void wifi_lnk_inf_for_iface(struct iface *iface, struct lq_wifi_dat **wifi) {
	int finish = 0;
	struct netlnk_msg *req_msg = NULL;
	struct netlnk_cb *req_cb = NULL;
	struct wifi_lnk_inf_context lnk_context = { &finish, wifi };

	ASSERT_NOT_NULL(iface);

	if (! iface->is_wireless) {
		// Remove in production code
		olsr_md_syslog(OLSR_MD_LOG_INF, "Link entry %s is not a wireless lnk", iface->int_name);
		return;
	}

	if ((req_msg = netlnkmsg_alloc()) == NULL) {
		olsr_md_exit("Failed to allocate netlnk_msg struct", EXIT_FAIL);
	}

	genetlnkmsg_put(req_msg, NETLNK_AUTO_PID, NETLNK_AUTO_SEQ, netlnk_id, 0, NLM_F_DUMP, WIFI_CMD_GET_STATION, 0);

	if (netlnka_put_u32(req_msg, WIFI_ATTR_IFINDEX, iface->if_index) == -1) {
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Failed to add iface index to netlnk msg");
		exit(1);
	}

#ifdef NETLNK_DEBUG
	if ((req_cb = netlnk_cb_alloc(NETLNK_CB_DEBUG)) == NULL) {
#else
	if ((req_cb = netlnk_cb_alloc(NETLNK_CB_DEFAULT)) == NULL) {
#endif
		olsr_md_exit("Failed to alloc netlnk_cb struct", EXIT_FAIL);
	}

	if (netlnk_cb_set(req_cb, NETLNK_CB_VALID, NETLNK_CB_CUSTOM, parse_wifi_msg, &lnk_context) != 0) {
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Failed to set netlnk msg callback");
		exit(1);
	}

	netlnk_cb_err(req_cb, NETLNK_CB_CUSTOM, error_hdlr, &finish);
	netlnk_cb_set(req_cb, NETLNK_CB_FINISH, NETLNK_CB_CUSTOM, finish_hdlr, &finish);
	netlnk_cb_set(req_cb, NETLNK_CB_ACK, NETLNK_CB_CUSTOM, ack_hdlr, &finish);

	if (netlnk_send_auto_complete(gen_netlnk_socket, req_msg) < 0) {
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Failed sending the req msg with netlnk");
		exit(1);
	}

	while (! finish) {
		netlnk_recvmsgs(gen_netlnk_socket, req_cb);
	}

	netlnkmsg_free(req_msg);
}

/**
 * Uses the os ARP cache to find a MAC addressess for a neighbor. Does not do
 * actual ARP if it's not found in the cache.
 *
 * @param lnk		Neighbor to find MAC addressess of.
 * @param mac		Pointer to buffer of size ETHERNET_ADDRESS_LEN that will be
 *					used to write MAC addressess in (if found).
 * @returns			True if MAC addressess is found.
 */
static bool mac_of_neighbor(struct lnk_entry *lnk, unsigned char *mac) {
	bool success = false;
	struct netlnk_cache *cache = NULL;
	struct rtnetlnk_neigh *neighbor = NULL;
	struct netlnk_address *neighbor_address_filter = NULL;
	struct netlnk_address *neighbor_mac_address = NULL;
	void *address = NULL;
	size_t address_size = 0;

	if (olsr_md_cnf->ip_version == AF_INET6) {
		address = &(lnk->neighbor_iface_address.v6);
		address_size = sizeof(struct in6_address);
	} else {
		address = &(lnk->neighbor_iface_address.v4);
		address_size = sizeof(struct in_address);
	}

	if ((neighbor_address_filter = netlnk_address_build(olsr_md_cnf->ip_version, address, address_size)) == NULL) {
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Failed to build netlnk_address struct from lnk ip addressess");
		goto cleanup;
	}

#if !defined(CONF_LIBNL20) && !defined(CONF_LIBNL30)
	if ((cache = rtnetlnk_neigh_alloc_cache(rt_netlnk_socket)) == NULL) {
#else
	if (rtnetlnk_neigh_alloc_cache(rt_netlnk_socket, &cache) != 0) {
#endif
		olsr_md_syslog(OLSR_MD_LOG_ERR, "Failed to allocate netlnk neighbor cache");
		goto cleanup;
	}

	if ((neighbor = rtnetlnk_neigh_get(cache, lnk->inter->if_index, neighbor_address_filter)) == NULL) {
		olsr_md_syslog(OLSR_MD_LOG_INF, "Neighbor MAC addressess not found in ARP cache");
		goto cleanup;
	}

	if ((neighbor_mac_address = rtnetlnk_neigh_get_lladdress(neighbor)) != NULL) {
		if (netlnk_address_get_len(neighbor_mac_address) != ETHERNET_ADDRESS_LEN) {
			olsr_md_syslog(OLSR_MD_LOG_ERR, "Found a netlnk nieghbor but addressess is not ETHERNET_ADDRESS_LEN long");
			goto cleanup;
		}
		memcpy(mac, netlnk_address_get_binary_address(neighbor_mac_address), ETHERNET_ADDRESS_LEN);
		success = true;
	}

cleanup:
	if (cache)
		netlnk_cache_free(cache);
	if (neighbor)
		rtnetlnk_neigh_put(neighbor);
	if (neighbor_address_filter)
		netlnk_address_put(neighbor_address_filter);
	// neighbor_mac_filter does not need cleanup

	return success;
}

void wifi_lnk_inf_init(void) {
	connect_netlnk();
}

void wifi_lnk_inf_cleanup(void) {
	netlnk_socket_free(gen_netlnk_socket);
	netlnk_socket_free(rt_netlnk_socket);
}

static void free_lq_wifi_dat(struct lq_wifi_dat *wifi) {
	struct lq_wifi_dat *next = wifi;
	while (wifi) {
		next = wifi->next;
		free(wifi);
		wifi = next;	
	}
}

/**
 * Find a object in the lnked list that matches the MAC addressess.
 *
 * @param wifi_list		Pointer to the lnked list to find the object in.
 * @param mac				MAC addressess to look for, MUST be ETHERNET_ADDRESS_LEN long.
 *
 * @returns					Pointer to object or NULL on failure.
 */
static struct lq_wifi_dat *find_lq_wifi_dat_by_mac(struct lq_wifi_dat *wifi_list, unsigned char *mac) {
	ASSERT_NOT_NULL(wifi_list);
	ASSERT_NOT_NULL(mac);

	while (wifi_list) {
		if (memcmp(mac, wifi_list->mac, ETHERNET_ADDRESS_LEN) == 0) {
			return wifi_list;
		}
		wifi_list = wifi_list->next;
	}

	return NULL;
}

static uint8_t band_to_qual(uint16_t band) {
	fpm ratio;
	fpm fp_band;
	fpm plty;

	if (band == 0) {
		return 0;
	}

	// BandPenalty = 1 - (Band / ReferenceBand)

	fp_band = itofpm(band);
	fp_band = fpmidiv(fp_band, 10); // 100Kbit/sec to Mbit/sec

	ratio = fpmidiv(fp_band, REF_BAND_MBIT_SEC);
	plty = fpmsub(itofpm(1), ratio);

	// Convert to 255 based number
	plty = fpmimul(plty, 255);

	return fpmtoi(plty);
}

static uint8_t sig_to_qual(int8_t sig) {
	// Map dBm levels to qual penalties
	struct sig_plty {
		int8_t sig;
		uint8_t plty; // 255=1.0
	};
	// Must be ordered
	static struct sig_plty sig_qual_tbl[] = {
		{ -75, 30 }, { -80, 60}, { -85, 120 }, { -90, 160 }, { -95, 200 }, { -100, 255 }
	};
	static size_t TABLE_SIZE = sizeof(sig_qual_tbl) / sizeof(struct sig_plty);

	unsigned int i = 0;
	uint8_t plty = 0;
	for (i = 0; i < TABLE_SIZE; i++) {
		if (sig <= sig_qual_tbl[i].sig) {
			plty = sig_qual_tbl[i].plty;
		} else {
			break;
		}
	}

	return plty;
}

void wifi_lnk_inf_get(void) {
	struct iface *next_iface = NULL;
	struct lq_wifi_dat *wifi_list = NULL;
	struct lnk_entry *lnk = NULL;
	struct lq_wifi_dat *lq_dat = NULL;
	struct lq_fetch_hello *lq_fetch = NULL;
	unsigned char mac_addressess[ETHERNET_ADDRESS_LEN];

	uint8_t plty_band;
	uint8_t plty_sig;

	// Get latest 802.11 status infrmation for all ifaces
	// This list will contain OLSR_MD and non-OLSR_MD nodes
	for (next_iface = ifnet; next_iface; next_iface = next_iface->int_next) {
		wifi_lnk_inf_for_iface(next_iface, &wifi_list);
	}

	if (wifi_list == NULL) {
		olsr_md_syslog(OLSR_MD_LOG_INF, "Failed to retreive any WIFI dat");
		return;
	}

	OLSR_MD_FOR_ALL_LNK_ENTRIES(lnk) {
		lq_fetch = (struct lq_fetch_hello *) lnk->lnkqual;
		lq_fetch->lq.valBand = 0;
		lq_fetch->lq.valRSSI = 0;
		lq_fetch->smoothed_lq.valBand = 0;
		lq_fetch->smoothed_lq.valRSSI = 0;

		if (mac_of_neighbor(lnk, mac_addressess)) {
			if ((lq_dat = find_lq_wifi_dat_by_mac(wifi_list, mac_addressess)) != NULL) {
				plty_band = band_to_qual(lq_dat->band);
				plty_sig = sig_to_qual(lq_dat->sig);

				lq_fetch->lq.valBand = plty_band;
				lq_fetch->lq.valRSSI = plty_sig;
				lq_fetch->smoothed_lq.valBand = plty_band;
				lq_fetch->smoothed_lq.valRSSI = plty_sig;

				olsr_md_syslog(OLSR_MD_LOG_INF, "Apply 802.11: iface(%s) neighbor(%s) band(%dMb = %d) rssi(%ddBm = %d)",
						lnk->if_name, ether_ntoa((struct ether_address *)mac_addressess),
						lq_dat->band / 10, plty_band, lq_dat->sig, plty_sig);
			} else
				olsr_md_syslog(OLSR_MD_LOG_INF, "NO match ;-(!");
		}
	} OLSR_MD_FOR_ALL_LNK_ENTRIES_END(lnk)

	free_lq_wifi_dat(wifi_list);
}

#endif /* OS_WIFI */

