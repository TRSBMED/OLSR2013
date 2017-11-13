/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/
#ifndef sr_hdm_h
#define sr_hdm_h

#include <assert.h>

#include <paquet.h>

#define SR_HDM_SZ 4			// taille de la partie constante de hdr

#define MAXI_SR_LEN 16		// la plus longue route source que nous pouvons gérer
#define MAXI_ROUTE_ERRORS 3	// combien d'erreurs de route peuvent tenir dans un paquet?

struct sr_address {
        int address_type;		// comme hdm_cmn dans paquet.h //
	nsaddress_t address;

	/*
	 * Métriques que je veux collecter à chaque nœud
	 */
	double	Pt_;
};

struct lnk_down {
	int address_type;		// comme hdm_cmn dans paquet.h 
	nsaddress_t tell_address;	// contact de l'hote
	nsaddress_t from_address;	// quand l'hote from_adrr ne peut plus
	nsaddress_t to_address;	// récupération des paquets de l'hote to_address
};


/* ======================================================================
   DSR Types de paquet
   ====================================================================== */
struct route_request {
	int	request_available_;	/* L'en-tête de la requête est-il availablee */
	int	request_id_;	/* identifiant de requête unique */
	int	request_ttl_;	/* propagation maximale */
};

struct route_reply {
	int	response_available_;	/* réponse en-tête est availablee? */
	int	response_rtlen_;	/* # sauts dans la réponse de route */
	struct sr_address	response_address_[MAXI_SR_LEN];
};

struct route_error {
	int	error_available_;	/* réponse en-tête est availablee? */
	int	error_count_;	/* nombre d'erreurs de route */
	struct lnk_down error_lnks_[MAXI_ROUTE_ERRORS];
};

/* ======================================================================
   DSR Prévision d'état de diffusion
   ====================================================================== */

struct flw_error {
	nsaddress_t  flw_src;
	nsaddress_t  flw_dst;
	u_int16_t flw_id; 
};

struct flw_header {
	int flw_available_;
	int hopCount_;
	unsigned short flw_id_;
};

struct flw_timeout {
	int flw_timeout_available_;
	unsigned long timeout_;  // timeout en secondes...
};

struct flw_unknown {
	int flw_unknown_available_;
	int error_count_;
	struct flw_error error_flws_[MAXI_ROUTE_ERRORS];
};

// flux par défaut, erreurs inconnues
struct flw_default_error {
	int flw_default_available_;
	int error_count_;
	struct flw_error error_flws_[MAXI_ROUTE_ERRORS];
};

struct bean {
	int seqno_;	//original uid
	int n_;	//le nombre de paquets qui sont divisés en
	int m_;	//le nombre de paquets nécessaires pour reconstruire le paquet original
	
};

/* ======================================================================
   DSR En-tete
   ====================================================================== */
class olsr_md_sr {
private:
	int available_;		/* cet en-tête est-il réellement dans le paquet? et initialisé? */
	int recovered_;	/* paquet a été récupéré? */

	int num_address_;
	int cur_address_;
	struct sr_address address_[MAXI_SR_LEN];

	struct route_request	sr_request_;
	struct route_reply	sr_reply_;
	struct route_error	sr_error_;

	struct flw_header      sr_flw_;
	struct flw_timeout	sr_ftime_;
	struct flw_unknown	sr_funk_;
	struct flw_default_error sr_fdef_unk;

	struct bean sr_bean_;

	int destination_;
	
public:
	static int offset_;		/* décalage pour cet en-tête */
	inline int& offset() { return offset_; }
        inline static olsr_md_sr* access(const paquet* p) {
	        return (olsr_md_sr*)p->access(offset_);
	}
	inline int& available() { return available_; }
	inline int& recovered() { return recovered_; }
	inline int& num_address() { return num_address_; }
	inline int& cur_address() { return cur_address_; }

	inline int available() const { return available_; }
	inline int recovered() const { return recovered_; }
	inline int num_address() const { return num_address_; }
	inline int cur_address() const { return cur_address_; }
	inline struct sr_address* address() { return address_; }

	inline int& route_request() {return sr_request_.request_available_; }
	inline int& rtrequest_seq() {return sr_request_.request_id_; }
	inline int& maxi_propagation() {return sr_request_.request_ttl_; }

	inline int& route_reply() {return sr_reply_.response_available_; }
	inline int& route_reply_len() {return sr_reply_.response_rtlen_; }
	inline struct sr_address* reply_address() {return sr_reply_.response_address_; }

	inline int& route_error() {return sr_error_.error_available_; }
	inline int& num_route_errors() {return sr_error_.error_count_; }
	inline struct lnk_down* down_lnks() {return sr_error_.error_lnks_; }

	inline int &flw_header() { return sr_flw_.flw_available_; }
	inline u_int16_t &flw_id() { return sr_flw_.flw_id_; }
	inline int &hopCount() { return sr_flw_.hopCount_; }

	inline int &flw_timeout() { return sr_ftime_.flw_timeout_available_; }
	inline unsigned long &flw_timeout_time() { return sr_ftime_.timeout_; }

	inline int &flw_unknown() { return sr_funk_.flw_unknown_available_; }
	inline int &num_flw_unknown() { return sr_funk_.error_count_; }
	inline struct flw_error *unknown_flws() { return sr_funk_.error_flws_; }

	inline int &flw_default_unknown() { return sr_fdef_unk.flw_default_available_; }
	inline int &num_default_unknown() { return sr_fdef_unk.error_count_; }
	inline struct flw_error *unknown_defaults() { return sr_fdef_unk.error_flws_; }

	//for bean
	inline int &seq_no(){return sr_bean_.seqno_;}
	inline int &m_(){return sr_bean_.m_;}
	inline int &n_(){return sr_bean_.n_;}

	//destination
	inline int &destination_node(){return destination_;}

	inline int size() {
		int sz = 0;
		if (num_address_ || route_request() || 
		    route_reply() || route_error() ||
		    flw_timeout() || flw_unknown() || flw_default_unknown())
			sz += SR_HDM_SZ;

		if (num_address_)			sz += 4 * (num_address_ - 1);
		if (route_reply())		sz += 5 + 4 * route_reply_len();
		if (route_request())		sz += 8;
		if (route_error())		sz += 16 * num_route_errors();
		if (flw_timeout())		sz += 4;
		if (flw_unknown())		sz += 14 * num_flw_unknown();
		if (flw_default_unknown())	sz += 12 * num_default_unknown();

		if (flw_header())		sz += 4;

		sz = ((sz+3)&(~3)); // alignement...
		assert(sz >= 0);
#if 0
		printf("Size: %d (%d %d %d %d %d %d %d %d %d)\n", sz,
			(num_address_ || route_request() ||
			route_reply() || route_error() ||
			flw_timeout() || flw_unknown() || 
			flw_default_unknown()) ? SR_HDM_SZ : 0,
			num_address_ ? 4 * (num_address_ - 1) : 0,
			route_reply() ? 5 + 4 * route_reply_len() : 0,
			route_request() ? 8 : 0,
			route_error() ? 16 * num_route_errors() : 0,
			flw_timeout() ? 4 : 0,
			flw_unknown() ? 14 * num_flw_unknown() : 0,
			flw_default_unknown() ? 12 * num_default_unknown() : 0,
			flw_header() ? 4 : 0);
#endif

		return sz;
	}

	inline nsaddress_t& get_next_address() { 
		assert(cur_address_ < num_address_);
		return (address_[cur_address_ + 1].address);
	}

	inline int& get_next_type() {
		assert(cur_address_ < num_address_);
		return (address_[cur_address_ + 1].address_type);
	}

	inline void append_address(nsaddress_t a, int type) {
		assert(num_address_ < MAXI_SR_LEN-1);
		address_[num_address_].address_type = type;
		address_[num_address_++].address = a;
	}

	inline void init() {
		available_ = 1;
		recovered_ = 0;
		num_address_ = 0;
		cur_address_ = 0;

		route_request() = 0;
		route_reply() = 0;
		route_reply_len() = 0;
		route_error() = 0;
		num_route_errors() = 0;

		flw_timeout() = 0;
		flw_unknown() = 0;
		flw_default_unknown() = 0;
		flw_header() = 0;
	}

#if 0
#ifdef DSR_CONST_HDM_SZ
  /* utilisé pour estimer l'avantage potentiel de retirer la route source dans chaque paquet */
	inline int size() { 
		return SR_HDM_SZ;
	}
#else
	inline int size() { 
		int sz = SR_HDM_SZ +
			4 * (num_address_ - 1) +
			4 * (route_reply() ? route_reply_len() : 0) +
			8 * (route_error() ? num_route_errors() : 0);
		assert(sz >= 0);
		return sz;
	}
#endif // DSR_CONST_HDM_SZ
#endif // 0

  void dump(char *);
  char* dump();
};


#endif // sr_hdm_h
