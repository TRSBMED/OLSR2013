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
/// \file	olsr_md_printer.cc
/// \brief	Printing functions used for debugging and tracing are implemented in this file.
///

#include "olsr_md/olsr_md_printer.h"
/********** Messages and paquets printing functions **********/

///
/// \brief Prints a given common header into a given file.
/// \warning This function is actually not used.
/// \param out	File where the common header is going to be written.
/// \param ch	Common header to be written.
///
void
olsr_md_printer::print_cmn_hdr(FILE* out, struct hdm_cmn* ch) {
	char *error, *direction;
	
	if (ch->error())
		error = "yes";
	else
		error = "no";
		
	if (ch->direction() == hdm_cmn::DOWN)
		direction = "DOWN";
	else if (ch->direction() == hdm_cmn::UP)
		direction = "UP";
	else if (ch->direction() == hdm_cmn::NONE)
		direction = "NONE";
	else
		direction = "UNKNOWN (likely a bug!)";
	
	// We could include this if we were interested in printing addressess type
	/*if (ch->address_type() == NS_AF_NONE)
		address_type = "NS_AF_NONE";
	else if (ch->address_type() == NS_AF_ILNK)
		address_type = "NS_AF_ILNK";
	else if (ch->address_type() == NS_AF_INET)
		address_type = "NS_AF_INET";
	else
		address_type = "UNKNOWN (possibly a bug!)";*/
	
	fprintf(out,
		"  COMMON HEADER\n\tptype = %s\n\tuid = %d\n\tsize = %d\n\terror = %s\n\tdirection = %s\n\n",
		paquet_info.name(ch->ptype()),
		ch->uid(),
		ch->size(),
		error,
		direction);
}

///
/// \brief Prints a given IP header into a given file.
/// \warning This function is actually not used.
/// \param out	File where the IP header is going to be written.
/// \param ih	IP header to be written.
///
void
olsr_md_printer::print_ip_hdr(FILE* out, struct hdm_ip* ih) {
	fprintf(out,
		"  IP HEADER\n\tsrc_address = %d\n\tsrc_port = %d\n\tdestination_address = %d\n\tdestination_port = %d\n\tttl = %d\n\n",
		ih->saddress(),
		ih->sport(),
		ih->daddress(),
		ih->dport(),
		ih->ttl());
}

///
/// \brief Prints a given olsr_md paquet into a given file.
/// \warning This function is actually not used.
/// \param out	File where the %olsr_md paquet is going to be written.
/// \param pqt	%olsr_md paquet to be written.
///
void
olsr_md_printer::print_olsr_md_pqt(FILE* out, olsr_md_pqt* pqt) {
		
	fprintf(out,
		"  olsr_md paquet\n\tlength = %d\n\tseq_num = %d\n\t------------\n",
		pqt->pqt_len(),
		pqt->pqt_seq_num());
	
	for (int i = 0; i < pqt->count; i++) {
		print_olsr_md_msg(out, pqt->msg(i));
		fprintf(out, "\t------------\n");
	}
	fprintf(out, "\n");
}

///
/// \brief Prints a given %olsr_md message into a given file.
/// \warning This function is actually not used.
/// \param out	File where the %olsr_md message is going to be written.
/// \param msg	%olsr_md message to be written.
///
void
olsr_md_printer::print_olsr_md_msg(FILE* out, olsr_md_msg& msg) {
	char *msg_type;
	
	switch (msg.msg_type()) {
		case olsr_md_HELLO_MSG:
			msg_type = "HELLO";
			break;
		case olsr_md_TC_MSG:
			msg_type = "TC";
			break;
		case olsr_md_MID_MSG:
			msg_type = "MID";
			break;
		default:
			msg_type = "UNKNOWN (likely a bug!)";
	}
	
	fprintf(out,
		"\ttype = %s\n\tvtime = %.2f\n\tmsg_size = %d\n\t"
		"orig_address = %d\n\tttl = %d\n\thop_count = %d\n\t"
		"msg_seq_num = %d\n",
		msg_type,
		olsr_md::emf_to_seconds(msg.vtime()),
		msg.msg_size(),
		msg.orig_address(),
		msg.ttl(),
		msg.hop_count(),
		msg.msg_seq_num());
	
	if (msg.msg_type() == olsr_md_HELLO_MSG)
		print_olsr_md_hello(out, msg.hello());
	else if (msg.msg_type() == olsr_md_TC_MSG)
		print_olsr_md_tc(out, msg.tc());
	else if (msg.msg_type() == olsr_md_MID_MSG)
		print_olsr_md_mid(out, msg.mid());
}

///
/// \brief Prints a given %olsr_md HELLO message into a given file.
/// \warning This function is actually not used.
/// \param out		File where the %olsr_md HELLO message is going to be written.
/// \param hello	%olsr_md HELLO message to be written.
///
void
olsr_md_printer::print_olsr_md_hello(FILE* out, olsr_md_hello& hello) {
	
	fprintf(out, "\thtime = %.2f\n\twillingness = %d\n",
		olsr_md::emf_to_seconds(hello.htime()),
		hello.willingness());
	
	for (int i = 0; i < hello.count; i++) {
		char *nt, *lt;
		olsr_md_hello_msg msg = hello.hello_msg(i);
		
		u_int8_t nb_type = msg.lnk_code() >> 2;
		u_int8_t lnk_type = msg.lnk_code() & 0x03;
		if (nb_type == olsr_md_NOT_NEIGH)
			nt = "NOT_NEIGH";
		else if (nb_type == olsr_md_SYM_NEIGH)
			nt = "SYM_NEIGH";
		else if (nb_type == olsr_md_MPR_NEIGH)
			nt = "MPR_NEIGH";
		else
			nt = "UNKNOWN (likely a bug!)";
		
		if (lnk_type == olsr_md_UNSPEC_LNK)
			lt = "UNSPEC_LNK";
		else if (lnk_type == olsr_md_ASYM_LNK)
			lt = "ASYM_LNK";
		else if (lnk_type == olsr_md_SYM_LNK)
			lt = "SYM_LNK";
		else if (lnk_type == olsr_md_LOST_LNK)
			lt = "LOST_LNK";
		else
			lt = "UNKNOWN (likely a bug!)";
		
		fprintf(out, "\tlnk_code = %s - %s\n\tlnk_msg_size = %d\n",
			nt,
			lt,
			msg.lnk_msg_size());
	
		for (int j = 0; j < msg.count; j++)
			fprintf(out, "\tnb_intface_address = %d\n", msg.nb_intface_address(j));
	}
}

///
/// \brief Prints a given %olsr_md TC message into a given file.
/// \warning This function is actually not used.
/// \param out	File where the %olsr_md TC message is going to be written.
/// \param tc	%olsr_md TC message to be written.
///
void
olsr_md_printer::print_olsr_md_tc(FILE* out, olsr_md_tc& tc) {
	fprintf(out, "\tansn = %d\n\treserved = %d\n",
		tc.ansn(),
		tc.reserved());
	for (int i = 0; i < tc.count; i++)
		fprintf(out, "\taddress = %d\n", tc.nb_main_address(i));
}

///
/// \brief Prints a given %olsr_md MID message into a given file.
/// \warning This function is actually not used.
/// \param out	File where the %olsr_md MID message is going to be written.
/// \param mid	%olsr_md MID message to be written.
///
void
olsr_md_printer::print_olsr_md_mid(FILE* out, olsr_md_mid& mid) {
	for (int i = 0; i < mid.count; i++)
		fprintf(out, "\tintface = %d\n", mid.intface_address(i));
}

/********** Repositories printing functions **********/

///
/// \brief Prints a given Link Set into a given trace file.
///
/// \param out		Trace where the Link Set is going to be written.
/// \param lnkset	Link Set to be written.
///
void
olsr_md_printer::print_lnkset(Trace* out, lnkset_t& lnkset) {
	sprintf(out->pt_->buffer(), "P\tlocal\tnb\tsym\t\tasym\t\tlost\t\ttime");
	out->pt_->dump();
	for (lnkset_t::iterator it = lnkset.begin(); it != lnkset.end(); it++) {
		olsr_md_lnk_tuple* tuple = *it;
		sprintf(out->pt_->buffer(), "P\t%d\t%d\t%f\t%f\t%f\t%f",
			olsr_md::node_id(tuple->local_intface_address()),
			olsr_md::node_id(tuple->nb_intface_address()),
			tuple->sym_time(),
			tuple->asym_time(),
			tuple->lost_time(),
			tuple->time());
		out->pt_->dump();
	}
}

///
/// \brief Prints a given Neighbor Set into a given trace file.
///
/// \param out		Trace where the Neighbor Set is going to be written.
/// \param nbset	Neighbor Set to be written.
///
void
olsr_md_printer::print_nbset(Trace* out, nbset_t& nbset) {
	sprintf(out->pt_->buffer(), "P\tnb\tstatus\twillingness");
	out->pt_->dump();
	for (nbset_t::iterator it = nbset.begin(); it != nbset.end(); it++) {
		olsr_md_nb_tuple* tuple = *it;
		sprintf(out->pt_->buffer(), "P\t%d\t%d\t%d",
			olsr_md::node_id(tuple->nb_main_address()),
			tuple->status(),
			tuple->willingness());
		out->pt_->dump();
	}
}

///
/// \brief Prints a given 2-hop Neighbor Set into a given trace file.
///
/// \param out		Trace where the 2-hop Neighbor Set is going to be written.
/// \param nb2hopset	2-hop Neighbor Set to be written.
///
void
olsr_md_printer::print_nb2hopset(Trace* out, nb2hopset_t& nb2hopset) {
	sprintf(out->pt_->buffer(), "P\tnb\tnb2hop\ttime");
	out->pt_->dump();
	for (nb2hopset_t::iterator it = nb2hopset.begin(); it != nb2hopset.end(); it++) {
		olsr_md_nb2hop_tuple* tuple = *it;
		sprintf(out->pt_->buffer(), "P\t%d\t%d\t%f",
			olsr_md::node_id(tuple->nb_main_address()),
			olsr_md::node_id(tuple->nb2hop_address()),
			tuple->time());
		out->pt_->dump();
	}
}

///
/// \brief Prints a given MPR Set into a given trace file.
///
/// \param out		Trace where the MPR Set is going to be written.
/// \param mprset	MPR Set to be written.
///
void
olsr_md_printer::print_mprset(Trace* out, mpr_set_t& mprset) {
	sprintf(out->pt_->buffer(), "P\tnb");
	out->pt_->dump();
	for (mpr_set_t::iterator it = mprset.begin(); it != mprset.end(); it++) {
		sprintf(out->pt_->buffer(), "P\t%d", olsr_md::node_id(*it));
		out->pt_->dump();
	}
}

///
/// \brief Prints a given MPR Selector Set into a given trace file.
///
/// \param out		Trace where the MPR Selector Set is going to be written.
/// \param mprselset	MPR Selector Set to be written.
///
void
olsr_md_printer::print_mprselset(Trace* out, mpr_selset_t& mprselset) {
	sprintf(out->pt_->buffer(), "P\tnb\ttime");
	out->pt_->dump();
	for (mpr_selset_t::iterator it = mprselset.begin(); it != mprselset.end(); it++) {
		olsr_md_mprsel_tuple* mprsel_tuple = *it;
		sprintf(out->pt_->buffer(), "P\t%d\t%f",
			olsr_md::node_id(mprsel_tuple->main_address()),
			mprsel_tuple->time());
		out->pt_->dump();
	}
}

///
/// \brief Prints a given Topology Set into a given trace file.
///
/// \param out		Trace where the Topology Set is going to be written.
/// \param topologyset	Topology Set to be written.
///
void
olsr_md_printer::print_topologyset(Trace* out, topologyset_t& topologyset) {
	sprintf(out->pt_->buffer(), "P\tdest\tlast\tseq\ttime");
	out->pt_->dump();
	for (topologyset_t::iterator it = topologyset.begin(); it != topologyset.end(); it++) {
		olsr_md_topology_tuple* topology_tuple = *it;
		sprintf(out->pt_->buffer(), "P\t%d\t%d\t%d\t%f",
			olsr_md::node_id(topology_tuple->destination_address()),
			olsr_md::node_id(topology_tuple->last_address()),
			topology_tuple->seq(),
			topology_tuple->time());
		out->pt_->dump();
	}
}
