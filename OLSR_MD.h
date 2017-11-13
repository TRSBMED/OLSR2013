/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

///
/// \file	olsr_md.h
/// \brief	Header file for olsr_md agent and related classes.
///
/// Here are defined all timers used by olsr_md, including those for managing internal
/// state and those for sending messages. Class olsr_md is also defined, therefore this
/// file has signatures for the most important methods. Lots of constants are also
/// defined.
///

#ifndef __olsr_md_h__
#define __olsr_md_h__

#include "olsr_md_pqt.h"
#include "olsr_md_state.h"
#include "olsr_md_rtable.h"
#include "olsr_md_m_rtbl.h"
#include "olsr_md_repositories.h"
#include "trace.h"
#include "classifier-port.h"
#include "agent.h"
#include "paquet.h"
#include "timer-handler.h"
#include "random.h"
#include "vector"
#include "bean.h"
#include <dsr-priqueue.h>

/********** Useful macros **********/

/// Returns maximum of two numbers.
#ifndef MAX
#define	MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/// Returns minimum of two numbers.
#ifndef MIN
#define	MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/// Defines NULL as zero, used for pointers.
#ifndef NULL
#define NULL 0
#endif

/// Gets current time from the scheduler.
#define CURRENT_TIME	Scheduler::instance().clock()

///
/// \brief Gets the delay between a given time and the current time.
///
/// If given time is previous to the current one, then this macro returns
/// a number close to 0. This is used for scheduling events at a certain moment.
///
#define DELAY(time) (((time) < (CURRENT_TIME)) ? (0.000001) : \
	(time - CURRENT_TIME + 0.000001))

/// Scaling factor used in RFC 3626.
#define OLSR_MD_C		0.0625


/********** Intervals **********/

/// HELLO messages emission interval.
#define OLSR_MD_HELLO_INTERVAL	 2

/// TC messages emission interval.
#define OLSR_MD_TC_INTERVAL	 5

/// MID messages emission interval.
#define OLSR_MD_MID_INTERVAL	OLSR_MD_TC_INTERVAL

///
/// \brief Period at which a node must cite every lnk and every neighbor.
///
/// We only use this value in order to define olsr_md_NEIGHB_HOLD_TIME.
///
#define OLSR_MD_REFRESH_INTERVAL	3


/********** Holding times **********/

/// Neighbor holding time.
#define OLSR_MD_NEIGHB_HOLD_TIME	3*OLSR_MD_REFRESH_INTERVAL
/// Top holding time.
#define OLSR_MD_TOP_HOLD_TIME	3*OLSR_MD_TC_INTERVAL
/// Dup holding time.
#define OLSR_MD_DUP_HOLD_TIME	30
/// MID holding time.
#define OLSR_MD_MID_HOLD_TIME	3*OLSR_MD_MID_INTERVAL


/********** Link types **********/

/// Unspecified lnk type.
#define OLSR_MD_UNSPEC_LNK	0
/// Asymmetric lnk type.
#define OLSR_MD_ASYM_LNK		1
/// Symmetric lnk type.
#define OLSR_MD_SYM_LNK		2
/// Lost lnk type.
#define OLSR_MD_LOST_LNK		3

/********** Neighbor types **********/

/// Not neighbor type.
#define OLSR_MD_NOT_NEIGH		0
/// Symmetric neighbor type.
#define OLSR_MD_SYM_NEIGH		1
/// Asymmetric neighbor type.
#define OLSR_MD_MPR_NEIGH		2


/********** Willingness **********/

/// Willingness for forwarding paquets from other nodes: never.
#define OLSR_MD_WILL_NEVER		0
/// Willingness for forwarding paquets from other nodes: low.
#define OLSR_MD_WILL_LOW		1
/// Willingness for forwarding paquets from other nodes: medium.
#define OLSR_MD_WILL_DEFAULT	3
/// Willingness for forwarding paquets from other nodes: high.
#define OLSR_MD_WILL_HIGH		6
/// Willingness for forwarding paquets from other nodes: always.
#define OLSR_MD_WILL_ALWAYS	7


/********** Miscellaneous constants **********/

/// Maximum allowed jitter.
#define OLSR_MD_MAXJITTER		OLSR_MD_HELLO_INTERVAL/4
/// Maximum allowed sequence number.
#define OLSR_MD_MAXI_SEQ_NUM	65535
/// Used to set status of an olsr_md_nb_tuple as "not symmetric".
#define OLSR_MD_STATUS_NOT_SYM	0
/// Used to set status of an olsr_md_nb_tuple as "symmetric".
#define OLSR_MD_STATUS_SYM		1
/// Random number between [0-olsr_md_MAXJITTER] used to jitter olsr_md paquet transmission.
#define JITTER			(Random::uniform()*OLSR_MD_MAXJITTER)

/*****************Type of Dijsktra node***********/
#define P_TYPE 1
#define T_TYPE 0

#define MAXI_ROUTE 3

#define n_ 4        //the paquets needed to do the coding
#define N_ 4		//the number of the projection
#define M_ 2		//the paquet needed to do the decoding

class olsr_md;			// forward declaration





/********** Timers **********/


/// Timer for sending an enqued message.
class olsr_md_MsgTimer : public TimerHandler {
public:
	olsr_md_MsgTimer(olsr_md* agent) : TimerHandler() {
		agent_	= agent;
	}
protected:
	olsr_md*	agent_;			///< olsr_md agent which created the timer.
	virtual void expire(Event* e);
};

/// Timer for sending HELLO messages.
class olsr_md_HelloTimer : public TimerHandler {
public:
	olsr_md_HelloTimer(olsr_md* agent) : TimerHandler() { agent_ = agent; }
protected:
	olsr_md*	agent_;			///< olsr_md agent which created the timer.
	virtual void expire(Event* e);
};


/// Timer for sending TC messages.
class olsr_md_TcTimer : public TimerHandler {
public:
	olsr_md_TcTimer(olsr_md* agent) : TimerHandler() { agent_ = agent; }
protected:
	olsr_md*	agent_;			///< olsr_md agent which created the timer.
	virtual void expire(Event* e);
};


/// Timer for sending MID messages.
class olsr_md_MidTimer : public TimerHandler {
public:
	olsr_md_MidTimer(olsr_md* agent) : TimerHandler() { agent_ = agent; }
protected:
	olsr_md*	agent_;			///< olsr_md agent which created the timer.
	virtual void expire(Event* e);
};


/// Timer for removing duplicate tuples: olsr_md_dup_tuple.
class olsr_md_DupTupleTimer : public TimerHandler {
public:
	olsr_md_DupTupleTimer(olsr_md* agent, olsr_md_dup_tuple* tuple) : TimerHandler() {
		agent_ = agent;
		tuple_ = tuple;
	}
protected:
	olsr_md*		agent_;	///< olsr_md agent which created the timer.
	olsr_md_dup_tuple*	tuple_;	///< olsr_md_dup_tuple which must be removed.
	
	virtual void expire(Event* e);
};


/// Timer for removing lnk tuples: olsr_md_lnk_tuple.
class olsr_md_LinkTupleTimer : public TimerHandler {
	///
	/// \brief A flag which tells if the timer has expired (at least) once or not.
	///
	/// When a lnk tuple has been just created, its sym_time is expired but this
	/// does not mean a neighbor loss. Thus, we use this flag in order to be able
	/// to distinguish this situation.
	///
	bool			first_time_;
public:
	olsr_md_LinkTupleTimer(olsr_md* agent, olsr_md_lnk_tuple* tuple) : TimerHandler() {
		agent_		= agent;
		tuple_		= tuple;
		first_time_	= true;
	}
protected:
	olsr_md*			agent_;	///< olsr_md agent which created the timer.
	olsr_md_lnk_tuple*	tuple_;	///< olsr_md_lnk_tuple which must be removed.
	
	virtual void expire(Event* e);
};


/// Timer for removing nb2hop tuples: olsr_md_nb2hop_tuple.
class olsr_md_Nb2hopTupleTimer : public TimerHandler {
public:
	olsr_md_Nb2hopTupleTimer(olsr_md* agent, olsr_md_nb2hop_tuple* tuple) : TimerHandler() {
		agent_ = agent;
		tuple_ = tuple;
	}
protected:
	olsr_md*			agent_;	///< olsr_md agent which created the timer.
	olsr_md_nb2hop_tuple*	tuple_;	///< olsr_md_nb2hop_tuple which must be removed.
	
	virtual void expire(Event* e);
};


/// Timer for removing MPR selector tuples: olsr_md_mprsel_tuple.
class olsr_md_MprSelTupleTimer : public TimerHandler {
public:
	olsr_md_MprSelTupleTimer(olsr_md* agent, olsr_md_mprsel_tuple* tuple) : TimerHandler() {
		agent_ = agent;
		tuple_ = tuple;
	}
protected:
	olsr_md*			agent_;	///< olsr_md agent which created the timer.
	olsr_md_mprsel_tuple*	tuple_;	///< olsr_md_mprsel_tuple which must be removed.
	
	virtual void expire(Event* e);
};


/// Timer for removing topology tuples: olsr_md_topology_tuple.
class olsr_md_TopologyTupleTimer : public TimerHandler {
public:
	olsr_md_TopologyTupleTimer(olsr_md* agent, olsr_md_topology_tuple* tuple) : TimerHandler() {
		agent_ = agent;
		tuple_ = tuple;
	}
protected:
	olsr_md*			agent_;	///< olsr_md agent which created the timer.
	olsr_md_topology_tuple*	tuple_;	///< olsr_md_topology_tuple which must be removed.
	
	virtual void expire(Event* e);
};


/// Timer for removing interface association tuples: olsr_md_intface_assoc_tuple.
class olsr_md_IfaceAssocTupleTimer : public TimerHandler {
public:
	olsr_md_IfaceAssocTupleTimer(olsr_md* agent, olsr_md_intface_assoc_tuple* tuple) : TimerHandler() {
		agent_ = agent;
		tuple_ = tuple;
	}
protected:
	olsr_md*			agent_;	///< olsr_md agent which created the timer.
	olsr_md_intface_assoc_tuple*	tuple_;	///< olsr_md_intface_assoc_tuple which must be removed.
	
	virtual void expire(Event* e);
};

class BeanTimer : public TimerHandler {

public :

    BeanTimer(int uid = 0);

    virtual void expire(Event *e);

protected :
    int mUid;
};


class Bean {
    
public :
    

    static std::map<int, paquet*> sOriginals;  //the original paquets for reconstruction, 1 to n

	static std::map<int, std::vector<paquet*> > sOriginals_N; //the original paquets for reconstruction, N to n 

    static std::map<int, BeanTimer> sTimers;

		std::map<int, std::vector< paquet*> > mSend;       //the send buffer, the key is the  destination


	Bean();
	~Bean();
	
	void test();

//the bean transform
    std::vector<paquet*> burst(paquet* pqt, int n, int m, 
            std::vector<std::vector<nsaddress_t> >* address);

	std::vector<paquet*> burst(paquet* pqt, int N, int n, int m, 
		std::vector<std::vector<nsaddress_t> >* address);

//the reverse transform
	

    paquet* rebuild(paquet* pqt);

	std::vector<paquet*> rebuild(paquet* pqt, int t);

private :

    std::map<int, int > mReceived;       //the receive counter, the key is the paquet ID

};



/********** olsr_md Agent **********/


///
/// \brief Routing agent which implements %olsr_md protocol following RFC 3626.
///
/// Interacts with TCL interface through command() method. It implements all
/// functionalities related to sending and receiving paquets and managing
/// internal state.
///
class olsr_md : public Tap, public Agent {
	// Makes some friends.
	friend class olsr_md_HelloTimer;
	friend class olsr_md_TcTimer;
	friend class olsr_md_MidTimer;
	friend class olsr_md_DupTupleTimer;
	friend class olsr_md_LinkTupleTimer;
	friend class olsr_md_Nb2hopTupleTimer;
	friend class olsr_md_MprSelTupleTimer;
	friend class olsr_md_TopologyTupleTimer;
	friend class olsr_md_IfaceAssocTupleTimer;
	friend class olsr_md_MsgTimer;
	friend class BeanTimer;
	//friend class Bean;

//	  LnkOutObject *ll;		        // our lnk layer output 
	  
	/// Address of the routing agent.
	nsaddress_t	ra_address_;
	
	/// Paquets sequence number counter.
	u_int16_t	pqt_seq_;
	/// Messages sequence number counter.
	u_int16_t	msg_seq_;
	/// Advertised Neighbor Set sequence number.
	u_int16_t	ansn_;
	
	/// HELLO messages' emission interval.
	int		hello_ival_;
	/// TC messages' emission interval.
	int		tc_ival_;
	/// MID messages' emission interval.
	int		mid_ival_;
	/// Willingness for forwarding paquets on behalf of other nodes.
	int		willingness_;
	/// Determines if layer 2 notifications are enabled or not.
	int		use_mac_;
	
	/// Routing table.
	olsr_md_rtable		rtable_;

	/// Multipath routing table
	olsr_md_m_rtbl		m_rtbl_;

	
	/// Internal state with all needed data structs.
	olsr_md_state		state_;
	/// A list of pending messages which are buffered awaiting for being sent.
	std::vector<olsr_md_msg>	msgs_;

	///paquet count
	int 		paquet_count_;

	///bean
	Bean		bean_;
	
protected:
	PortClassifier*	dmux_;		///< For passing paquets up to agents.
	Trace*		logtarget_;	///< For logging.
	
	olsr_md_HelloTimer	hello_timer_;	///< Timer for sending HELLO messages.
	olsr_md_TcTimer	tc_timer_;	///< Timer for sending TC messages.
	olsr_md_MidTimer	mid_timer_;	///< Timer for sending MID messages.

	//BeanTimer moj_timer_;
	/// Increments paquet sequence number and returns the new value.
	inline u_int16_t	pqt_seq() {
		pqt_seq_ = (pqt_seq_ + 1)%(olsr_md_MAXI_SEQ_NUM + 1);
		return pqt_seq_;
	}
	/// Increments message sequence number and returns the new value.
	inline u_int16_t	msg_seq() {
		msg_seq_ = (msg_seq_ + 1)%(olsr_md_MAXI_SEQ_NUM + 1);
		return msg_seq_;
	}
	
	inline nsaddress_t&	ra_address()	{ return ra_address_; }
	
	inline int&		hello_ival()	{ return hello_ival_; }
	inline int&		tc_ival()	{ return tc_ival_; }
	inline int&		mid_ival()	{ return mid_ival_; }
	inline int&		willingness()	{ return willingness_; }
	inline int&		use_mac()	{ return use_mac_; }
	
	inline lnkset_t&	lnkset()	{ return state_.lnkset(); }
	inline mpr_set_t&	mprset()	{ return state_.mprset(); }
	inline mpr_selset_t&	mprselset()	{ return state_.mprselset(); }
	inline nbset_t&		nbset()		{ return state_.nbset(); }
	inline nb2hopset_t&	nb2hopset()	{ return state_.nb2hopset(); }
	inline topologyset_t&	topologyset()	{ return state_.topologyset(); }
	inline dupset_t&	dupset()	{ return state_.dupset(); }
	inline intfaceassocset_t&	intfaceassocset()	{ return state_.intfaceassocset(); }
	
	void		recv_olsr_md(paquet*);
	
	void		mpr_computation();
	void		rtable_computation();
	void 		m_rtbl_computation(paquet*);

	void		process_hello(olsr_md_msg&, nsaddress_t, nsaddress_t);
	void		process_tc(olsr_md_msg&, nsaddress_t);
	void		process_mid(olsr_md_msg&, nsaddress_t);
	
	void		forward_default(paquet*, olsr_md_msg&, olsr_md_dup_tuple*, nsaddress_t);
	void		forward_data(paquet*);
	void 		m_forward_data(paquet*);
	void		resend_data(paquet*);
	
	void		enque_msg(olsr_md_msg&, double);
	void		send_hello();
	void		send_tc();
	void		send_mid();
	void		send_pqt();
	
	void		lnk_sensing(olsr_md_msg&, nsaddress_t, nsaddress_t);
	void		populate_nbset(olsr_md_msg&);
	void		populate_nb2hopset(olsr_md_msg&);
	void		populate_mprselset(olsr_md_msg&);

	void		set_hello_timer();
	void		set_tc_timer();
	void		set_mid_timer();

	void		nb_loss(olsr_md_lnk_tuple*);
	void		add_dup_tuple(olsr_md_dup_tuple*);
	void		rm_dup_tuple(olsr_md_dup_tuple*);
	void		add_lnk_tuple(olsr_md_lnk_tuple*, u_int8_t);
	void		rm_lnk_tuple(olsr_md_lnk_tuple*);
	void		updated_lnk_tuple(olsr_md_lnk_tuple*);
	void		add_nb_tuple(olsr_md_nb_tuple*);
	void		rm_nb_tuple(olsr_md_nb_tuple*);
	void		add_nb2hop_tuple(olsr_md_nb2hop_tuple*);
	void		rm_nb2hop_tuple(olsr_md_nb2hop_tuple*);
	void		add_mprsel_tuple(olsr_md_mprsel_tuple*);
	void		rm_mprsel_tuple(olsr_md_mprsel_tuple*);
	void		add_topology_tuple(olsr_md_topology_tuple*);
	void		rm_topology_tuple(olsr_md_topology_tuple*);
	void		add_intfaceassoc_tuple(olsr_md_intface_assoc_tuple*);
	void		rm_intfaceassoc_tuple(olsr_md_intface_assoc_tuple*);
	
	nsaddress_t	get_main_address(nsaddress_t);
	int		degree(olsr_md_nb_tuple*);

	static bool	seq_num_bigger_than(u_int16_t, u_int16_t);
	float 		tuple_weight(float a);
	
	LnkOutObject *ll;		        // our lnk layer output 
	CMUPrimaryQueue *ifq;		// output interface queue
	Mac *mac_;

public:
	olsr_md(nsaddress_t);
	int	command(int, const char*const*);
	void	recv(paquet*, Handler*);
	void	mac_failed(paquet*);
	
	static double		emf_to_seconds(u_int8_t);
	static u_int8_t		seconds_to_emf(double);
	static int		node_id(nsaddress_t);

	  void tap(const paquet *p);
  // tap out all data paquets received at this host and promiscously snoop
  // them for interesting tidbits
};




#endif
