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
/// \file	olsr_md_state.h
/// \brief	This header file declares and defines internal state of an olsr_md node.
///

#ifndef __olsr_md_state_h__
#define __olsr_md_state_h__

#include "olsr_md_repositories.h"

/// This class encapsulates all data structures needed for maintaining internal state of an olsr_md node.
class olsr_md_state {
	friend class olsr_md;
	
	lnkset_t	lnkset_;	///< Link Set (RFC 3626, section 4.2.1).
	nbset_t		nbset_;		///< Neighbor Set (RFC 3626, section 4.3.1).
	nb2hopset_t	nb2hopset_;	///< 2-hop Neighbor Set (RFC 3626, section 4.3.2).
	topologyset_t	topologyset_;	///< Topology Set (RFC 3626, section 4.4).
	mpr_set_t	mprset_;	///< MPR Set (RFC 3626, section 4.3.3).
	mpr_selset_t	mprselset_;	///< MPR Selector Set (RFC 3626, section 4.3.4).
	dupset_t	dupset_;	///< Duplicate Set (RFC 3626, section 3.4).
	intfaceassocset_t	intfaceassocset_;	///< Interface Association Set (RFC 3626, section 4.1).
	
protected:
	inline	lnkset_t&		lnkset()	{ return lnkset_; }
	inline	mpr_set_t&		mprset()	{ return mprset_; }
	inline	mpr_selset_t&		mprselset()	{ return mprselset_; }
	inline	nbset_t&		nbset()		{ return nbset_; }
	inline	nb2hopset_t&		nb2hopset()	{ return nb2hopset_; }
	inline	topologyset_t&		topologyset()	{ return topologyset_; }
	inline	dupset_t&		dupset()	{ return dupset_; }
	inline	intfaceassocset_t&	intfaceassocset()	{ return intfaceassocset_; }
	
	olsr_md_mprsel_tuple*	find_mprsel_tuple(nsaddress_t);
	void			erase_mprsel_tuple(olsr_md_mprsel_tuple*);
	void			erase_mprsel_tuples(nsaddress_t);
	void			insert_mprsel_tuple(olsr_md_mprsel_tuple*);
	
	olsr_md_nb_tuple*		find_nb_tuple(nsaddress_t);
	olsr_md_nb_tuple*		find_sym_nb_tuple(nsaddress_t);
	olsr_md_nb_tuple*		find_nb_tuple(nsaddress_t, u_int8_t);
	void			erase_nb_tuple(olsr_md_nb_tuple*);
	void			erase_nb_tuple(nsaddress_t);
	void			insert_nb_tuple(olsr_md_nb_tuple*);
	
	olsr_md_nb2hop_tuple*	find_nb2hop_tuple(nsaddress_t, nsaddress_t);
	void			erase_nb2hop_tuple(olsr_md_nb2hop_tuple*);
	void			erase_nb2hop_tuples(nsaddress_t);
	void			erase_nb2hop_tuples(nsaddress_t, nsaddress_t);
	void			insert_nb2hop_tuple(olsr_md_nb2hop_tuple*);

	bool			find_mpr_address(nsaddress_t);
	void			insert_mpr_address(nsaddress_t);
	void			clear_mprset();
	
	olsr_md_dup_tuple*		find_dup_tuple(nsaddress_t, u_int16_t);
	void			erase_dup_tuple(olsr_md_dup_tuple*);
	void			insert_dup_tuple(olsr_md_dup_tuple*);
	
	olsr_md_lnk_tuple*	find_lnk_tuple(nsaddress_t);
	olsr_md_lnk_tuple*	find_sym_lnk_tuple(nsaddress_t, double);
	void			erase_lnk_tuple(olsr_md_lnk_tuple*);
	void			insert_lnk_tuple(olsr_md_lnk_tuple*);

	olsr_md_topology_tuple*	find_topology_tuple(nsaddress_t, nsaddress_t);
	olsr_md_topology_tuple*	find_newer_topology_tuple(nsaddress_t, u_int16_t);
	void			erase_topology_tuple(olsr_md_topology_tuple*);
	void			erase_older_topology_tuples(nsaddress_t, u_int16_t);
	void			insert_topology_tuple(olsr_md_topology_tuple*);
	
	olsr_md_intface_assoc_tuple* find_intfaceassoc_tuple(nsaddress_t);
	void			erase_intfaceassoc_tuple(olsr_md_intface_assoc_tuple*);
	void			insert_intfaceassoc_tuple(olsr_md_intface_assoc_tuple*);
	
};

#endif
