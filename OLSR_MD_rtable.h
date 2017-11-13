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
/// \file	olsr_md_rtable.h
/// \brief	Header file for routing table's related stuff.
///

#ifndef __olsr_md_rtable_h__
#define __olsr_md_rtable_h__

#include "olsr_md_repositories.h"
#include "trace.h"
#include "map"

///
/// \brief Defines rtable_t as a map of olsr_md_rt_entry, whose key is the destination addressess.
///
/// The routing table is thus defined as pairs: [dest addressess, entry]. Each element
/// of the pair can be accesed via "first" and "second" members.
///
typedef std::map<nsaddress_t, olsr_md_rt_entry*> rtable_t;

///
/// \brief This class is a representation of the olsr_md's Routing Table.
///
class olsr_md_rtable {
	rtable_t	rt_;	///< Data structure for the routing table.

public:
	
	olsr_md_rtable();
	~olsr_md_rtable();
	
	void		clear();
	void		rm_entry(nsaddress_t dest);
	olsr_md_rt_entry*	add_entry(nsaddress_t dest, nsaddress_t next, nsaddress_t intface, u_int32_t dist);
	olsr_md_rt_entry*	lookup(nsaddress_t dest);
	olsr_md_rt_entry*	find_send_entry(olsr_md_rt_entry*);
	u_int32_t	size();
	void		print(Trace*);
};

#endif
