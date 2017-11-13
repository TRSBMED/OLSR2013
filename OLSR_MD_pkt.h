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
/// \file	olsr_md_pqt.h
/// \brief	This file contains all declarations of %olsr_md paquets and messages,
///		including all related contants and macros.
///

#ifndef __olsr_md_pqt_h__
#define __olsr_md_pqt_h__

#include "paquet.h"

/********** Message types **********/

/// %OLSR_MD HELLO message type.
#define OLSR_MD_HELLO_MSG		1
/// %OLSR_MD TC message type.
#define OLSR_MD_TC_MSG		2
/// %OLSR_MD MID message type.
#define OLSR_MD_MID_MSG		3

/********** Paquets stuff **********/

/// Accessor to the %olsr_md paquet.
#define PKT_OLSR_MD(p)		olsr_md_pqt::access(p)

///
/// \brief Size of the addressesses we are using.
///
/// You should always use this macro when interested in calculate addressesses'
/// sizes. By default IPv4 addressesses are supposed, but you can change to
/// IPv6 ones by recompiling with olsr_md_IPv6 constant defined.
///
#ifdef OLSR_MD_IPv6
#define ADDR_SIZE	16
#else
#define ADDR_SIZE	4
#endif

/// Maximum number of messages per paquet.
#define OLSR_MD_MAXI_MSGS		4

/// Maximum number of hellos per message (4 possible lnk types * 3 possible nb types).
#define OLSR_MD_MAXI_HELLOS		12

/// Maximum number of addressesses advertised on a message.
#define OLSR_MD_MAXI_ADDRS		64

/// Size (in bytes) of paquet header.
#define OLSR_MD_PKT_HDM_SIZE	4

/// Size (in bytes) of message header.
#define OLSR_MD_MSG_HDM_SIZE	12

/// Size (in bytes) of hello header.
#define OLSR_MD_HELLO_HDM_SIZE	4

/// Size (in bytes) of hello_msg header.
#define OLSR_MD_HELLO_MSG_HDM_SIZE	4

/// Size (in bytes) of tc header.
#define OLSR_MD_TC_HDM_SIZE	4

/// Auxiliary struct which is part of the %OLSR_MD HELLO message (struct olsr_md_hello).
typedef struct olsr_md_hello_msg {

        /// Link code.
	u_int8_t	lnk_code_;
	/// Reserved.
	u_int8_t	reserved_;
	/// Size of this lnk message.
	u_int16_t	lnk_msg_size_;
	/// List of interface addressesses of neighbor nodes.
	nsaddress_t	nb_intface_address_[OLSR_MD_MAXI_ADDRS];
	/// Number of interface addressesses contained in nb_intface_address_.
	int		count;
	
	inline u_int8_t&	lnk_code()		{ return lnk_code_; }
	inline u_int8_t&	reserved()		{ return reserved_; }
	inline u_int16_t&	lnk_msg_size()		{ return lnk_msg_size_; }
	inline nsaddress_t&	nb_intface_address(int i)	{ return nb_intface_address_[i]; }
	
	inline u_int32_t size() { return OLSR_MD_HELLO_MSG_HDM_SIZE + count*ADDR_SIZE; }

} olsr_md_hello_msg;

/// %olsr_md HELLO message.
typedef struct olsr_md_hello {

	/// Reserved.
	u_int16_t	reserved_;
	/// HELLO emission interval in mantissa/exponent format.
	u_int8_t	htime_;
	/// Willingness of a node for forwarding paquets on behalf of other nodes.
	u_int8_t	willingness_;
	/// List of olsr_md_hello_msg.
	olsr_md_hello_msg	hello_body_[OLSR_MD_MAXI_HELLOS];
	/// Number of olsr_md_hello_msg contained in hello_body_.
	int		count;
	
	inline u_int16_t&	reserved()		{ return reserved_; }
	inline u_int8_t&	htime()			{ return htime_; }
	inline u_int8_t&	willingness()		{ return willingness_; }
	inline olsr_md_hello_msg&	hello_msg(int i)	{ return hello_body_[i]; }
	
	inline u_int32_t size() {
		u_int32_t sz = OLSR_MD_HELLO_HDM_SIZE;
		for (int i = 0; i < count; i++)
			sz += hello_msg(i).size();
		return sz;
	}
	
} olsr_md_hello;

/// %olsr_md TC message.
typedef struct olsr_md_tc {

        /// Advertised Neighbor Sequence Number.
	u_int16_t	ansn_;
	/// Reserved.
	u_int16_t	reserved_;
	/// List of neighbors' main addressesses.
	nsaddress_t	nb_main_address_[OLSR_MD_MAXI_ADDRS];
	/// Number of neighbors' main addressesses contained in nb_main_address_.
	int		count;
        
	inline	u_int16_t&	ansn()			{ return ansn_; }
	inline	u_int16_t&	reserved()		{ return reserved_; }
	inline	nsaddress_t&	nb_main_address(int i)	{ return nb_main_address_[i]; }
	
	inline	u_int32_t size() { return OLSR_MD_TC_HDM_SIZE + count*ADDR_SIZE; }

} olsr_md_tc;

/// %olsr_md MID message.
typedef struct olsr_md_mid {

	/// List of interface addressesses.
	nsaddress_t	intface_address_[OLSR_MD_MAXI_ADDRS];
	/// Number of interface addressesses contained in intface_address_.
	int		count;
	
	inline nsaddress_t&	intface_address(int i)	{ return intface_address_[i]; }
	
	inline u_int32_t	size()			{ return count*ADDR_SIZE; }
	
} olsr_md_mid;

/// %olsr_md message.
typedef struct olsr_md_msg {

	u_int8_t	msg_type_;	///< Message type.
	u_int8_t	vtime_;		///< Validity time.
	u_int16_t	msg_size_;	///< Message size (in bytes).
	nsaddress_t	orig_address_;	///< Main addressess of the node which generated this message.
	u_int8_t	ttl_;		///< Time to live (in hops).
	u_int8_t	hop_count_;	///< Number of hops which the message has taken.
	u_int16_t	msg_seq_num_;	///< Message sequence number.
	union {
		olsr_md_hello	hello_;
		olsr_md_tc		tc_;
		olsr_md_mid	mid_;
	} msg_body_;			///< Message body.
	
	inline	u_int8_t&	msg_type()	{ return msg_type_; }
	inline	u_int8_t&	vtime()		{ return vtime_; }
	inline	u_int16_t&	msg_size()	{ return msg_size_; }
	inline	nsaddress_t&	orig_address()	{ return orig_address_; }
	inline	u_int8_t&	ttl()		{ return ttl_; }
	inline	u_int8_t&	hop_count()	{ return hop_count_; }
	inline	u_int16_t&	msg_seq_num()	{ return msg_seq_num_; }
	inline	olsr_md_hello&	hello()		{ return msg_body_.hello_; }
	inline	olsr_md_tc&	tc()		{ return msg_body_.tc_; }
	inline	olsr_md_mid&	mid()		{ return msg_body_.mid_; }
	
	inline u_int32_t size() {
		u_int32_t sz = OLSR_MD_MSG_HDM_SIZE;
		if (msg_type() == OLSR_MD_HELLO_MSG)
			sz += hello().size();
		else if (msg_type() == OLSR_MD_TC_MSG)
			sz += tc().size();
		else if (msg_type() == OLSR_MD_MID_MSG)
			sz += mid().size();
		return sz;
	}

} olsr_md_msg;

/// %olsr_md paquet.
typedef struct olsr_md_pqt {

	u_int16_t	pqt_len_;			///< paquet length (in bytes).
	u_int16_t	pqt_seq_num_;			///< paquet sequence number.
	olsr_md_msg	pqt_body_[olsr_md_MAXI_MSGS];	///< paquet body.
	int		count;				///< Number of olsr_md_msg contained in pqt_body_.
	
	inline	u_int16_t&	pqt_len()	{ return pqt_len_; }
	inline	u_int16_t&	pqt_seq_num()	{ return pqt_seq_num_; }
	inline	olsr_md_msg&	msg(int i)	{ return pqt_body_[i]; }
	
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static struct olsr_md_pqt* access(const paquet* p) {
		return (struct olsr_md_pqt*)p->access(offset_);
	}

} olsr_md_pqt;

#endif
