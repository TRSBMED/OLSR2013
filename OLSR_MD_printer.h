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
/// \file	olsr_md_printer.h
/// \brief	Header file which includes all printing functions related to olsr_md.
///

#ifndef __olsr_md_printer_h__
#define __olsr_md_printer_h__

#include "olsr_md.h"
#include "olsr_md_pqt.h"
#include "olsr_md_repositories.h"
#include "paquet.h"
#include "ip.h"
#include "trace.h"

/// Encapsulates all printing functions for olsr_md data structures and messages.
class olsr_md_printer {
	friend class olsr_md;

protected:
	static void	print_lnkset(Trace*, lnkset_t&);
	static void	print_nbset(Trace*, nbset_t&);
	static void	print_nb2hopset(Trace*, nb2hopset_t&);
	static void	print_mprset(Trace*, mpr_set_t&);
	static void	print_mprselset(Trace*, mpr_selset_t&);
	static void	print_topologyset(Trace*, topologyset_t&);
	
	static void	print_olsr_md_pqt(FILE*, olsr_md_pqt*);
	static void	print_olsr_md_msg(FILE*, olsr_md_msg&);
	static void	print_olsr_md_hello(FILE*, olsr_md_hello&);
	static void	print_olsr_md_tc(FILE*, olsr_md_tc&);
	static void	print_olsr_md_mid(FILE*, olsr_md_mid&);

public:
	static void	print_cmn_hdr(FILE*, struct hdm_cmn*);
	static void	print_ip_hdr(FILE*, struct hdm_ip*);
};

#endif
