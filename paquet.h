/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifndef ns_paquet_h
#define ns_paquet_h

#include <string.h>
#include <assert.h>

#include "config.h"
#include "scheduler.h"
#include "object.h"
#include "lib/bsd-list.h"
#include "paquet-stamp.h"
#include "ns-process.h"

// Utilisé par le code de routage sans fil pour attacher l'agent de routage
#define ROUT_PORT		255	/ * port ou tous les messages sont envoyés à * /

#define HDM_CMN(p)      (hdm_cmn::access(p))
#define HDM_ARP(p)      (hdm_arp::access(p))
#define HDM_MAC(p)      (hdm_mac::access(p))
#define HDM_MAC802_11(p) ((hdm_mac802_11 *)hdm_mac::access(p))
#define HDM_MAC_TDMA(p) ((hdm_mac_tdma *)hdm_mac::access(p))
#define HDM_SMAC(p)     ((hdm_smac *)hdm_mac::access(p))
#define HDM_LL(p)       (hdm_ll::access(p))
#define HDM_HDLC(p)     ((hdm_hdlc *)hdm_ll::access(p))
#define HDM_IP(p)       (hdm_ip::access(p))
#define HDM_RTP(p)      (hdm_rtp::access(p))
#define HDM_TCP(p)      (hdm_tcp::access(p))
#define HDM_SCTP(p)     (hdm_sctp::access(p))
#define olsr_md_sr(p)   (olsr_md_sr::access(p))
#define HDM_TFRC(p)     (hdm_tfrc::access(p))
#define HDM_TORA(p)     (hdm_tora::access(p))
#define HDM_IMEP(p)     (hdm_imep::access(p))
#define HDM_CDIFF(p)    (hdm_cdiff::access(p))  
#define HDM_LMS(p)	(hdm_lms::access(p))

enum paquet_t {
	PQT_TCP,
	PQT_UDP,
	PQT_CBR,
	PQT_AUDIO,
	PQT_VIDEO,
	PQT_ACK,
	PQT_START,
	PQT_STOP,
	PQT_PRUNE,
	PQT_GRAFT,
	PQT_GRAFTACK,
	PQT_JOIN,
	PQT_ASSERT,
	PQT_MESSAGE,
	PQT_RTCP,
	PQT_RTP,
	PQT_RTPROTO_DV,
	PQT_CtrMcast_Encap,
	PQT_CtrMcast_Decap,
	PQT_SRM,
	PQT_REQUEST,	
	PQT_ACCEPT,	
	PQT_CONFIRM,	
	PQT_TEARDOWN,	
	PQT_LIVE,	// paquet du réseau en direct
	PQT_REJECT,
	PQT_TELNET,	// non nécessaire: telnet utilise TCP
	PQT_FTP,
	PQT_PARETO,
	PQT_EXP,
	PQT_INVAL,
	PQT_HTTP,
	/* nouvel encapsulateur */
	PQT_ENCAPSULATED,
	PQT_MFTP,
	PQT_ARP,
	PQT_MAC,
	PQT_TORA,
	PQT_DSR,
	PQT_AODV,
	PQT_IMEP,
	// RAP paquets
	PQT_RAP_DATA,
	PQT_RAP_ACK,

	PQT_TFRC,
	PQT_TFRC_ACK,
	PQT_PING,

	// Diffusion paquets
	PQT_DIFF,

	// Mise à jour des paquets de routage
	PQT_RTPROTO_LS,

	// MPLS LDP en tete
	PQT_LDP,

	// GAF paquet
        PQT_GAF,  

	// ReadAudio traffic
	PQT_REALAUDIO,

	// Messages de refoulement
	PQT_PUSHBACK,

#ifdef HAVM_STL
	PQT_PGM,
#endif 
	PQT_LMS,
	PQT_LMS_SETUP,

	PQT_SCTP,
	PQT_SCTP_APP1,

	PQT_SMAC,

	PQT_XCP,
	
	PQT_HDLC,
	
	PQT_OLSR,

	PQT_OLSR_MD,
	
	// insérer de nouveaux types de paquet ici
	PQT_NTYPE // Celui-ci DOIT être le dernier
};

class p_infdm {
public:
	p_infdm() {
		name_[PQT_TCP]= "tcp";
		name_[PQT_UDP]= "udp";
		name_[PQT_CBR]= "cbr";
		name_[PQT_AUDIO]= "audio";
		name_[PQT_VIDEO]= "video";
		name_[PQT_ACK]= "ack";
		name_[PQT_START]= "start";
		name_[PQT_STOP]= "stop";
		name_[PQT_PRUNE]= "prune";
		name_[PQT_GRAFT]= "graft";
		name_[PQT_GRAFTACK]= "graftAck";
		name_[PQT_JOIN]= "join";
		name_[PQT_ASSERT]= "assert";
		name_[PQT_MESSAGE]= "message";
		name_[PQT_RTCP]= "rtcp";
		name_[PQT_RTP]= "rtp";
		name_[PQT_RTPROTO_DV]= "rtProtoDV";
		name_[PQT_CtrMcast_Encap]= "CtrMcast_Encap";
		name_[PQT_CtrMcast_Decap]= "CtrMcast_Decap";
		name_[PQT_SRM]= "SRM";

		name_[PQT_REQUEST]= "sa_req";	
		name_[PQT_ACCEPT]= "sa_accept";
		name_[PQT_CONFIRM]= "sa_conf";
		name_[PQT_TEARDOWN]= "sa_teardown";
		name_[PQT_LIVE]= "live"; 
		name_[PQT_REJECT]= "sa_reject";

		name_[PQT_TELNET]= "telnet";
		name_[PQT_FTP]= "ftp";
		name_[PQT_PARETO]= "pareto";
		name_[PQT_EXP]= "exp";
		name_[PQT_INVAL]= "httpInval";
		name_[PQT_HTTP]= "http";
		name_[PQT_ENCAPSULATED]= "encap";
		name_[PQT_MFTP]= "mftp";
		name_[PQT_ARP]= "ARP";
		name_[PQT_MAC]= "MAC";
		name_[PQT_TORA]= "TORA";
		name_[PQT_DSR]= "DSR";
		name_[PQT_AODV]= "AODV";
		name_[PQT_IMEP]= "IMEP";

		name_[PQT_RAP_DATA] = "rap_data";
		name_[PQT_RAP_ACK] = "rap_ack";

 		name_[PQT_TFRC]= "tcpFriend";
		name_[PQT_TFRC_ACK]= "tcpFriendCtl";
		name_[PQT_PING]="ping";

	 	/* Pour diffusion */
 		name_[PQT_DIFF] = "diffusion";

		// Mises à jour de routage d'état de liaison
		name_[PQT_RTPROTO_LS] = "rtProtoLS";

		name_[PQT_LDP] = "LDP";

                name_[PQT_GAF] = "gaf";      

		name_[PQT_REALAUDIO] = "ra";

		name_[PQT_PUSHBACK] = "pushback";

#ifdef HAVM_STL
		name_[PQT_PGM] = "PGM";
#endif 
		name_[PQT_LMS]="LMS";
		name_[PQT_LMS_SETUP]="LMS_SETUP";

		name_[PQT_SCTP]= "sctp";
 		name_[PQT_SCTP_APP1] = "sctp_app1";

		name_[PQT_SMAC]="smac";

		name_[PQT_HDLC]="HDLC";

		name_[PQT_XCP]="xcp";

		name_[PQT_OLSR]= "OLSR";

		name_[PQT_OLSR_MD] = "olsr_md";
		
		name_[PQT_NTYPE]= "undefined";
	}
	const char* name(paquet_t p) const { 
		if ( p <= PQT_NTYPE ) return name_[p];
		return 0;
	}
	static bool data_paquet(paquet_t type) {
		return ( (type) == PQT_TCP || \
			 (type) == PQT_TELNET || \
			 (type) == PQT_CBR || \
			 (type) == PQT_AUDIO || \
			 (type) == PQT_VIDEO || \
			 (type) == PQT_ACK || \
			 (type) == PQT_SCTP || \
			 (type) == PQT_SCTP_APP1 || \
			 (type) == PQT_HDLC \
			);
	}
private:
	static char* name_[PQT_NTYPE+1];
};
extern p_infdm paquet_info; 

#define DATA_PAQUET(type) ( (type) == PQT_TCP || \
                            (type) == PQT_TELNET || \
                            (type) == PQT_CBR || \
                            (type) == PQT_AUDIO || \
                            (type) == PQT_VIDEO || \
                            (type) == PQT_ACK || \
                            (type) == PQT_SCTP || \
                            (type) == PQT_SCTP_APP1 \
                            )


#define OFFSET(type, field)	((long) &((type *)0)->field)

class PaquetData : public AppData {
public:
	PaquetData(int sz) : AppData(PAQUET_DATA) {
		datalen_ = sz;
		if (datalen_ > 0)
			data_ = new unsigned char[datalen_];
		else
			data_ = NULL;
	}
	PaquetData(PaquetData& d) : AppData(d) {
		datalen_ = d.datalen_;
		if (datalen_ > 0) {
			data_ = new unsigned char[datalen_];
			memcpy(data_, d.data_, datalen_);
		} else
			data_ = NULL;
	}
	virtual ~PaquetData() { 
		if (data_ != NULL) 
			delete []data_; 
	}
	unsigned char* data() { return data_; }

	virtual int size() const { return datalen_; }
	virtual AppData* copy() { return new PaquetData(*this); }
private:
	unsigned char* data_;
	int datalen_;
};

typedef void (*FailureCallback)(paquet *,void *);

class paquet : public Event {
private:
	unsigned char* bits_;	
	static void init(paquet*);
	bool fflag_;
protected:
	static paquet* free_;	// Liste de paquets libre
	int	ref_count_;	// libérer les paquets jusqu'a 0
public:
	paquet* next_;		// pour les files d'attente et la liste libre
	static int hdrlen_;

	paquet() : bits_(0), data_(0), ref_count_(0), next_(0) { }
	inline unsigned char* const bits() { return (bits_); }
	inline paquet* copy() const;
	inline paquet* refcopy() { ++ref_count_; return this; }
	inline int& ref_count() { return (ref_count_); }
	static inline paquet* alloc();
	static inline paquet* alloc(int);
	inline void allocdata(int);
	// bidouille pour les données de diffusion
	inline void initdata() { data_  = 0;}
	static inline void free(paquet*);
	inline unsigned char* access(int off) const {
		if (off < 0)
			abort();
		return (&bits_[off]);
	}
	// Ceci est utilisé pour la rétrocompatibilité,
	// c'est-à-dire en supposant que les données de
	// l'utilisateur sont PaquetData et renvoyées vers son pointeur.
	inline unsigned char* accessdata() const { 
		if (data_ == 0)
			return 0;
		assert(data_->type() == PAQUET_DATA);
		return (((PaquetData*)data_)->data()); 
	}
	// Ceci est utilisé pour accéder à des données spécifiques
	// à l'application, non limitées à PaquetData.
	inline AppData* userdata() const {
		return data_;
	}
	inline void setdata(AppData* d) { 
		if (data_ != NULL)
			delete data_;
		data_ = d; 
	}
	inline int datalen() const { return data_ ? data_->size() : 0; }

	static void dump_header(paquet *p, int offset, int length);

	// le tampon du paquet transporte toutes les informations 
	// sur comment/où le paquet a été envoyé pour un récepteur
	// de déterminer s'il reçoit correctement le paquet
        PaquetStamp	txinfo_;  

	/*
         * Selon le code cmu:
	 * Ce flag est défini par la couche MAC sur un paquet entrant
	 * et est effacé par la couche de liaison. C'est un hack, mais
	 * il n'y a vraiment pas d'autre moyen parce que NS appelle
	 * toujours la fonction recv() d'un objet.
	 */
        u_int8_t        incoming;
};

/* 
 * associations de constantes statiques entre les valeurs spéciales
 * (négatives) de l'interface et leurs représentations de chaînes
 * de caractères utilisées par tcl
 */
class intface_literal {
public:
	enum intface_constant { 
		UNKN_INTFACE= -1, /* 
				   * valeur interne pour les paquets d'origine locale
				   */
		ANY_INTFACE= -2   /* 
				   * hashnode avec iif == ANY_INTFACE_ correspond à
				   * n'importe quelle interface de paquet (importée de TCL);
				   * cette valeur doit être différente de 
				   * hdm_cmn :: insu_INTFACE (paquet.h)
				   */ 
	};
	intface_literal(const intface_constant i, const char * const n) : 
		value_(i), name_(n) {}
	inline int value() const { return value_; }
	inline const char * const name() const { return name_; }
private:
	const intface_constant value_;
	/* chaînes utilisées dans TCL pour accéder à ces valeurs spéciales */
	const char * const name_; 
};

static const intface_literal UNKN_INTFACE(intface_literal::UNKN_INTFACE, "?");
static const intface_literal ANY_INTFACE(intface_literal::ANY_INTFACE, "*");

/*
 * Notez que NS_AF_ * ne correspond pas nécessairement aux constantes
 * utilisées dans votre système (car de nombreux systèmes n'ont ni NONE ni ILNK).
 */
enum ns_af_enum { NS_AF_NONE, NS_AF_ILNK, NS_AF_INET };

struct hdm_cmn {
	enum dir_t { DOWN= -1, NONE= 0, UP= 1 };
	paquet_t ptype_;	// type de paquet
	int	size_;		// taille du paquet simulé
	int	uid_;		// unique id
	int	error_;		// flag erreur
	int     errbitcnt_;     // bits corrompus
	int     fecsize_;
	double	ts_;		// timestamp: mesure de q-delay
	int	intface_;	// interface de réception
	dir_t	direction_;	// direction: 0=none, 1=up, -1=down 
        char src_rt_available;
	double ts_arr_; 	

	nsaddress_t prev_hop_;     // Adresse IP du transfert
	nsaddress_t next_hop_;	// prochain saut pour ce paquet
	int      address_type_;    // type de next_hop_ address
	nsaddress_t last_hop_;     // pour tracer sur des canaux multi-utilisateurs

        // Appelé si le paquet ne peut pas obtenir de support ou n'est pas acquitté.
        // Pas appelé s'il est déposé par une file d'attente
        FailureCallback xmint_failure_; 
        void *xmint_failure_data_;
        int     xmint_reason_;
#define XMINT_REASON_RTS 0x01
#define XMINT_REASON_ACK 0x02

        // rempli par GOD à la première transmission, utilisé pour l'analyse de traces
        int num_forwards_;	// combien de fois ce paquet a été transmis
        int opt_num_forwards_;  // renvoi optimal 

	double txtime_;
	inline double& txtime() { return(txtime_); }

	static int offset_;	// offset for this header
	inline static int& offset() { return offset_; }
	inline static hdm_cmn* access(const paquet* p) {
		return (hdm_cmn*) p->access(offset_);
	}
	
        /* Fonctions membre */
	inline paquet_t& ptype() { return (ptype_); }
	inline int& size() { return (size_); }
	inline int& uid() { return (uid_); }
	inline int& error() { return error_; }
	inline int& errbitcnt() {return errbitcnt_; }
	inline int& fecsize() {return fecsize_; }
	inline double& timestamp() { return (ts_); }
	inline int& intface() { return (intface_); }
	inline dir_t& direction() { return (direction_); }

	inline nsaddress_t& next_hop() { return (next_hop_); }
	inline int& address_type() { return (address_type_); }
	inline int& num_forwards() { return (num_forwards_); }
	inline int& opt_num_forwards() { return (opt_num_forwards_); }
};


class PaquetHeaderCls : public TclClass {
protected:
	PaquetHeaderCls(const char* classname, int hdrsize);
	virtual int method(int argc, const char*const* argv);
	void field_offset(const char* fieldname, int offset);
	inline void bind_offset(int* off) { offset_ = off; }
	inline void offset(int* off) {offset_= off;}
	int hdrlen_;		// # bytes pour l'entete
	int* offset_;		// offset pour l'entete
public:
	virtual void bind();
	virtual void export_offsets();
	TclObject* create(int argc, const char*const* argv);
};


inline void paquet::init(paquet* p)
{
	bzero(p->bits_, hdrlen_);
}

inline paquet* paquet::alloc()
{
	paquet* p = free_;
	if (p != 0) {
		assert(p->fflag_ == FALSE);
		free_ = p->next_;
		assert(p->data_ == 0);
		p->uid_ = 0;
		p->time_ = 0;
	} else {
		p = new paquet;
		p->bits_ = new unsigned char[hdrlen_];
		if (p == 0 || p->bits_ == 0)
			abort();
	}
	init(p); // Initialisation bits_[]
	(HDM_CMN(p))->next_hop_ = -2; // -1 reservé pour IP_BROADCAST
	(HDM_CMN(p))->last_hop_ = -2; // -1 reservé pour IP_BROADCAST
	p->fflag_ = TRUE;
	(HDM_CMN(p))->direction() = hdm_cmn::DOWN;
	/* mettre tous les paquets en direction de la descente par défaut;
	 * jusqu'à ce que le canal le modifie à +1 (vers le haut)
	 */
	p->next_ = 0;
	return (p);
}

/* 
 * Allouer un tampon de données n octets à un paquet existant
 * 
 * Pour définir AppData spécifique à l'application, utilisez paquet :: setdata ()
 */
inline void paquet::allocdata(int n)
{
	assert(data_ == 0);
	data_ = new PaquetData(n);
	if (data_ == 0)
		abort();
}

/* allouer un paquet avec un tampon de données n octets */
inline paquet* paquet::alloc(int n)
{
	paquet* p = alloc();
	if (n > 0) 
		p->allocdata(n);
	return (p);
}


inline void paquet::free(paquet* p)
{
	if (p->fflag_) {
		if (p->ref_count_ == 0) {
			/*
			 * L'UID d'un paquet peut être <0 (en dehors d'une file
			 * d'attente d'événements), ou == 0 (nouveau mais n'entrera
			 * jamais dans la file d'attente des événements.
			 */
			assert(p->uid_ <= 0);
			// Supprimez les données utilisateur car nous n'en aurons plus besoin.
			if (p->data_ != 0) {
				delete p->data_;
				p->data_ = 0;
			}
			init(p);
			p->next_ = free_;
			free_ = p;
			p->fflag_ = FALSE;
		} else {
			--p->ref_count_;
		}
	}
}

inline paquet* paquet::copy() const
{
	
	paquet* p = alloc();
	memcpy(p->bits(), bits_, hdrlen_);
	if (data_) 
		p->data_ = data_->copy();
	p->txinfo_.init(&txinfo_);
 
	return (p);
}

inline void
paquet::dump_header(paquet *p, int offset, int length)
{
        assert(offset + length <= p->hdrlen_);
        struct hdm_cmn *ch = HDM_CMN(p);

        fprintf(stderr, "\nPaquet ID: %d\n", ch->uid());

        for(int i = 0; i < length ; i+=16) {
                fprintf(stderr, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        p->bits_[offset + i],     p->bits_[offset + i + 1],
                        p->bits_[offset + i + 2], p->bits_[offset + i + 3],
                        p->bits_[offset + i + 4], p->bits_[offset + i + 5],
                        p->bits_[offset + i + 6], p->bits_[offset + i + 7],
                        p->bits_[offset + i + 8], p->bits_[offset + i + 9],
                        p->bits_[offset + i + 10], p->bits_[offset + i + 11],
                        p->bits_[offset + i + 12], p->bits_[offset + i + 13],
                        p->bits_[offset + i + 14], p->bits_[offset + i + 15]);
        }
}

#endif
