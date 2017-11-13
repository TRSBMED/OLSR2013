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
/// \file	olsr_md_state.cc
/// \brief	Implementation of all functions needed for manipulating the internal
///		state of an olsr_md node.
///

#include "olsr_md/olsr_md_state.h"
#include "olsr_md/olsr_md.h"

/********** MPR Selector Set Manipulation **********/

olsr_md_mprsel_tuple*
olsr_md_state::find_mprsel_tuple(nsaddress_t main_address) {
	for (mpr_selset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++) {
		olsr_md_mprsel_tuple* tuple = *it;
		if (tuple->main_address() == main_address)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_mprsel_tuple(olsr_md_mprsel_tuple* tuple) {
	for (mpr_selset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++) {
		if (*it == tuple) {
			mprselset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::erase_mprsel_tuples(nsaddress_t main_address) {
	for (mpr_selset_t::iterator it = mprselset_.begin(); it != mprselset_.end(); it++) {
		olsr_md_mprsel_tuple* tuple = *it;
		if (tuple->main_address() == main_address) {
			it = mprselset_.erase(it);
			it--;
		}
	}
}

void
olsr_md_state::insert_mprsel_tuple(olsr_md_mprsel_tuple* tuple) {
	mprselset_.push_back(tuple);
}

/********** Neighbor Set Manipulation **********/

olsr_md_nb_tuple*
olsr_md_state::find_nb_tuple(nsaddress_t main_address) {
	for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++) {
		olsr_md_nb_tuple* tuple = *it;
		if (tuple->nb_main_address() == main_address)
			return tuple;
	}
	return NULL;
}

olsr_md_nb_tuple*
olsr_md_state::find_sym_nb_tuple(nsaddress_t main_address) {
	for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++) {
		olsr_md_nb_tuple* tuple = *it;
		if (tuple->nb_main_address() == main_address && tuple->status() == olsr_md_STATUS_SYM)
			return tuple;
	}
	return NULL;
}

olsr_md_nb_tuple*
olsr_md_state::find_nb_tuple(nsaddress_t main_address, u_int8_t willingness) {
	for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++) {
		olsr_md_nb_tuple* tuple = *it;
		if (tuple->nb_main_address() == main_address && tuple->willingness() == willingness)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_nb_tuple(olsr_md_nb_tuple* tuple) {
	for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++) {
		if (*it == tuple) {
			nbset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::erase_nb_tuple(nsaddress_t main_address) {
	for (nbset_t::iterator it = nbset_.begin(); it != nbset_.end(); it++) {
		olsr_md_nb_tuple* tuple = *it;
		if (tuple->nb_main_address() == main_address) {
			it = nbset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::insert_nb_tuple(olsr_md_nb_tuple* tuple) {
	nbset_.push_back(tuple);
}

/********** Neighbor 2 Hop Set Manipulation **********/

olsr_md_nb2hop_tuple*
olsr_md_state::find_nb2hop_tuple(nsaddress_t nb_main_address, nsaddress_t nb2hop_address) {
	for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++) {
		olsr_md_nb2hop_tuple* tuple = *it;
		if (tuple->nb_main_address() == nb_main_address && tuple->nb2hop_address() == nb2hop_address)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_nb2hop_tuple(olsr_md_nb2hop_tuple* tuple) {
	for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++) {
		if (*it == tuple) {
			nb2hopset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::erase_nb2hop_tuples(nsaddress_t nb_main_address, nsaddress_t nb2hop_address) {
	for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++) {
		olsr_md_nb2hop_tuple* tuple = *it;
		if (tuple->nb_main_address() == nb_main_address && tuple->nb2hop_address() == nb2hop_address) {
			it = nb2hopset_.erase(it);
			it--;
		}
	}
}

void
olsr_md_state::erase_nb2hop_tuples(nsaddress_t nb_main_address) {
	for (nb2hopset_t::iterator it = nb2hopset_.begin(); it != nb2hopset_.end(); it++) {
		olsr_md_nb2hop_tuple* tuple = *it;
		if (tuple->nb_main_address() == nb_main_address) {
			it = nb2hopset_.erase(it);
			it--;
		}
	}
}

void
olsr_md_state::insert_nb2hop_tuple(olsr_md_nb2hop_tuple* tuple){
	nb2hopset_.push_back(tuple);
}

/********** MPR Set Manipulation **********/

bool
olsr_md_state::find_mpr_address(nsaddress_t address) {
	mpr_set_t::iterator it = mprset_.find(address);
	return (it != mprset_.end());
}

void
olsr_md_state::insert_mpr_address(nsaddress_t address) {
	mprset_.insert(address);
}

void
olsr_md_state::clear_mprset() {
	mprset_.clear();
}

/********** Duplicate Set Manipulation **********/

olsr_md_dup_tuple*
olsr_md_state::find_dup_tuple(nsaddress_t address, u_int16_t seq_num) {
	for (dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++) {
		olsr_md_dup_tuple* tuple = *it;
		if (tuple->address() == address && tuple->seq_num() == seq_num)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_dup_tuple(olsr_md_dup_tuple* tuple) {
	for (dupset_t::iterator it = dupset_.begin(); it != dupset_.end(); it++) {
		if (*it == tuple) {
			dupset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::insert_dup_tuple(olsr_md_dup_tuple* tuple) {
	dupset_.push_back(tuple);
}

/********** Link Set Manipulation **********/

olsr_md_lnk_tuple*
olsr_md_state::find_lnk_tuple(nsaddress_t intface_address) {
	for (lnkset_t::iterator it = lnkset_.begin(); it != lnkset_.end(); it++) {
		olsr_md_lnk_tuple* tuple = *it;
		if (tuple->nb_intface_address() == intface_address)
			return tuple;
	}
	return NULL;
}

olsr_md_lnk_tuple*
olsr_md_state::find_sym_lnk_tuple(nsaddress_t intface_address, double now) {
	for (lnkset_t::iterator it = lnkset_.begin(); it != lnkset_.end(); it++) {
		olsr_md_lnk_tuple* tuple = *it;
		if (tuple->nb_intface_address() == intface_address) {
			if (tuple->sym_time() > now)
				return tuple;
			else
				break;
		}
	}
	return NULL;
}

void
olsr_md_state::erase_lnk_tuple(olsr_md_lnk_tuple* tuple) {
	for (lnkset_t::iterator it = lnkset_.begin(); it != lnkset_.end(); it++) {
		if (*it == tuple) {
			lnkset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::insert_lnk_tuple(olsr_md_lnk_tuple* tuple) {
	lnkset_.push_back(tuple);
}

/********** Topology Set Manipulation **********/

olsr_md_topology_tuple*
olsr_md_state::find_topology_tuple(nsaddress_t destination_address, nsaddress_t last_address) {
	for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++) {
		olsr_md_topology_tuple* tuple = *it;
		if (tuple->destination_address() == destination_address && tuple->last_address() == last_address)
			return tuple;
	}
	return NULL;
}

olsr_md_topology_tuple*
olsr_md_state::find_newer_topology_tuple(nsaddress_t last_address, u_int16_t ansn) {
	for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++) {
		olsr_md_topology_tuple* tuple = *it;
		if (tuple->last_address() == last_address && tuple->seq() > ansn)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_topology_tuple(olsr_md_topology_tuple* tuple) {
	for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++) {
		if (*it == tuple) {
			topologyset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::erase_older_topology_tuples(nsaddress_t last_address, u_int16_t ansn) {
	for (topologyset_t::iterator it = topologyset_.begin(); it != topologyset_.end(); it++) {
		olsr_md_topology_tuple* tuple = *it;
		if (tuple->last_address() == last_address && tuple->seq() < ansn) {
			it = topologyset_.erase(it);
			it--;
		}
	}
}

void
olsr_md_state::insert_topology_tuple(olsr_md_topology_tuple* tuple) {
	topologyset_.push_back(tuple);
}

/********** Interface Association Set Manipulation **********/

olsr_md_intface_assoc_tuple*
olsr_md_state::find_intfaceassoc_tuple(nsaddress_t intface_address) {
	for (intfaceassocset_t::iterator it = intfaceassocset_.begin();
		it != intfaceassocset_.end();
		it++) {
		olsr_md_intface_assoc_tuple* tuple = *it;
		if (tuple->intface_address() == intface_address)
			return tuple;
	}
	return NULL;
}

void
olsr_md_state::erase_intfaceassoc_tuple(olsr_md_intface_assoc_tuple* tuple) {
	for (intfaceassocset_t::iterator it = intfaceassocset_.begin();
		it != intfaceassocset_.end();
		it++) {
		if (*it == tuple) {
			intfaceassocset_.erase(it);
			break;
		}
	}
}

void
olsr_md_state::insert_intfaceassoc_tuple(olsr_md_intface_assoc_tuple* tuple) {
	intfaceassocset_.push_back(tuple);
}
