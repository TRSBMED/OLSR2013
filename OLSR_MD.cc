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
/// \file	olsr_md.cc
/// \brief	Implementation of olsr_md agent and related classes.
///
/// This is the main file of this software because %olsr_md's behaviour is
/// implemented here.
///

#include "olsr_md/olsr_md.h"
#include "olsr_md/olsr_md_pqt.h"
#include "olsr_md/olsr_md_printer.h"
#include "olsr_md/olsr_md_sr.h"
//#include "olsr_md/MDpaquet.h"
#include "math.h"
#include "limits.h"
#include "addressess.h"
#include "ip.h"
#include "cmu-trace.h"
#include "map"
#include "set"
#include <iostream>

using namespace std;

/// Length (in bytes) of UDP header.
#define UDP_HDM_LEN	8

///
/// \brief Function called by MAC layer when cannot deliver a packet.
///
/// \param p Packet which couldn't be delivered.
/// \param arg olsr_md agent passed for a callback.
///
static void
olsr_md_mac_failed_callback(paquet *p, void *arg) {
  ((olsr_md*)arg)->mac_failed(p);
}


/********** TCL Hooks **********/


int olsr_md_pqt::offset_;
static class olsr_mdHeaderClass : public PaquetHeaderCls {
public:
	olsr_mdHeaderClass() : PaquetHeaderCls("PaquetHeader/olsr_md", sizeof(olsr_md_pqt)) {
		bind_offset(&olsr_md_pqt::offset_);
	}
} class_rtProtoolsr_md_hdr;

static class olsr_mdClass : public TclClass {
public:
	olsr_mdClass() : TclClass("Agent/olsr_md") {}
	TclObject* create(int argc, const char*const* argv) {
		// argv has the following structure:
		// <tcl-object> <tcl-object> Agent/olsr_md create-shadow <id>
		// e.g: _o17 _o17 Agent/olsr_md create-shadow 0
		// argv[4] is the address of the node
		assert(argc == 5);
		return new olsr_md((nsaddress_t)Address::instance().str2address(argv[4]));
	}
} class_rtProtoolsr_md;

///
/// \brief Interface with TCL interpreter.
///
/// From your TCL scripts or shell you can invoke commands on this olsr_md
/// routing agent thanks to this function. Currently you can call "start",
/// "print_rtable", "print_lnkset", "print_nbset", "print_nb2hopset",
/// "print_mprset", "print_mprselset" and "print_topologyset" commands.
///
/// \param argc Number of arguments.
/// \param argv Arguments.
/// \return TCL_OK or TCL_ERROR.
///
int
olsr_md::command(int argc, const char*const* argv) {
	if (argc == 2) {
		// Starts all timers
		if (strcasecmp(argv[1], "start") == 0) {
			hello_timer_.resched(0.0);
			tc_timer_.resched(0.0);
			mid_timer_.resched(0.0);
			
			return TCL_OK;
    		}
		// Prints routing table
		else if (strcasecmp(argv[1], "print_rtable") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Routing Table",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				rtable_.print(logtarget_);
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this routing table "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints lnk set
		else if (strcasecmp(argv[1], "print_lnkset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Link Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_lnkset(logtarget_, lnkset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this lnk set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints neighbor set
		else if (strcasecmp(argv[1], "print_nbset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Neighbor Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_nbset(logtarget_, nbset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this neighbor set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints 2-hop neighbor set
		else if (strcasecmp(argv[1], "print_nb2hopset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Neighbor2hop Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_nb2hopset(logtarget_, nb2hopset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this neighbor2hop set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints MPR set
		else if (strcasecmp(argv[1], "print_mprset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ MPR Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_mprset(logtarget_, mprset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this mpr set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints MPR selector set
		else if (strcasecmp(argv[1], "print_mprselset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ MPR Selector Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_mprselset(logtarget_, mprselset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this mpr selector set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
		// Prints topology set
		else if (strcasecmp(argv[1], "print_topologyset") == 0) {
			if (logtarget_ != NULL) {
				sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Topology Set",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
				logtarget_->pt_->dump();
				olsr_md_printer::print_topologyset(logtarget_, topologyset());
			}
			else {
				fprintf(stdout, "%f _%d_ If you want to print this topology set "
					"you must create a trace file in your tcl script",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()));
			}
			return TCL_OK;
		}
	}
	else if (argc == 3) {
		// Obtains the corresponding dmux to carry packet to upper layers
		if (strcmp(argv[1], "port-dmux") == 0) {
    			dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
			if (dmux_ == NULL) {
				fprintf(stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1], argv[2]);
				return TCL_ERROR;
			}
			return TCL_OK;
    		}
		// Obtains the corresponding tracer
		else if (strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
			logtarget_ = (Trace*)TclObject::lookup(argv[2]);
			if (logtarget_ == NULL)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
      	else if (strcasecmp(argv[1], "add-ll") == 0) {
		TclObject* obj;
		  if( (obj = TclObject::lookup(argv[2])) == 0) {
		    fprintf(stderr, "DSRAgent: %s lookup of %s failed\n", argv[1],
			    argv[2]);
		    return TCL_ERROR;
		  }
		  ll = (LnkOutObject*) obj;
		  if( (obj = TclObject::lookup(argv[3])) == 0) {
		    fprintf(stderr, "DSRAgent: %s lookup of %s failed\n", argv[1],
			    argv[3]);
		    return TCL_ERROR;
		  }
		  ifq = (CMUPrimaryQueue *) obj;
		  return TCL_OK;

	}
	// Pass the command up to the base class
	return Agent::command(argc, argv);
}


/********** Timers **********/


///
/// \brief Sends a HELLO message and reschedules the HELLO timer.
/// \param e The event which has expired.
///
void
olsr_md_HelloTimer::expire(Event* e) {
	agent_->send_hello();
	agent_->set_hello_timer();
}

///
/// \brief Sends a TC message (if there exists any MPR selector) and reschedules the TC timer.
/// \param e The event which has expired.
///
void
olsr_md_TcTimer::expire(Event* e) {
	if (agent_->mprselset().size() > 0)
		agent_->send_tc();
	agent_->set_tc_timer();
}

///
/// \brief Sends a MID message (if the node has more than one interface) and resets the MID timer.
/// \warning Currently it does nothing because there is no support for multiple interfaces.
/// \param e The event which has expired.
///
void
olsr_md_MidTimer::expire(Event* e) {
#ifdef MULTIPLE_INTFACES_SUPPORT
	agent_->send_mid();
	agent_->set_mid_timer();
#endif
}

///
/// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_DupTupleTimer::expire(Event* e) {
	if (tuple_->time() < CURRENT_TIME) {
		agent_->rm_dup_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else
		resched(DELAY(tuple_->time()));
}

///
/// \brief Removes tuple_ if expired. Else if symmetric time
/// has expired then it is assumed a neighbor loss and agent_->nb_loss()
/// is called. In this case the timer is rescheduled to expire at
/// tuple_->time(). Otherwise the timer is rescheduled to expire at
/// the minimum between tuple_->time() and tuple_->sym_time().
///
/// The task of actually removing the tuple is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_LinkTupleTimer::expire(Event* e) {
	double now	= CURRENT_TIME;
	
	if (tuple_->time() < now) {
		agent_->rm_lnk_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else if (tuple_->sym_time() < now) {
		if (first_time_)
			first_time_ = false;
		else
			agent_->nb_loss(tuple_);
		resched(DELAY(tuple_->time()));
	}
	else
		resched(DELAY(MIN(tuple_->time(), tuple_->sym_time())));
}

///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_Nb2hopTupleTimer::expire(Event* e) {
	if (tuple_->time() < CURRENT_TIME) {
		agent_->rm_nb2hop_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else
		resched(DELAY(tuple_->time()));
}

///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_MprSelTupleTimer::expire(Event* e) {
	if (tuple_->time() < CURRENT_TIME) {
		agent_->rm_mprsel_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else
		resched(DELAY(tuple_->time()));
}

///
/// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
///
/// The task of actually removing the tuple is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_TopologyTupleTimer::expire(Event* e) {
	if (tuple_->time() < CURRENT_TIME) {
		agent_->rm_topology_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else
		resched(DELAY(tuple_->time()));
}

///
/// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
/// \warning Actually this is never invoked because there is no support for multiple interfaces.
/// \param e The event which has expired.
///
void
olsr_md_IfaceAssocTupleTimer::expire(Event* e) {
	if (tuple_->time() < CURRENT_TIME) {
		agent_->rm_intfaceassoc_tuple(tuple_);
		delete tuple_;
		delete this;
	}
	else
		resched(DELAY(tuple_->time()));
}

///
/// \brief Sends a control packet which must bear every message in the olsr_md agent's buffer.
///
/// The task of actually sending the packet is left to the olsr_md agent.
///
/// \param e The event which has expired.
///
void
olsr_md_MsgTimer::expire(Event* e) {
	agent_->send_pqt();
	delete this;
}


/********** olsr_md class **********/


///
/// \brief Creates necessary timers, binds TCL-available variables and do
/// some more initializations.
/// \param id Identifier for the olsr_md agent. It will be used as the address
/// of this routing agent.
///
olsr_md::olsr_md(nsaddress_t id) :	Agent(PQT_OLSR_MD),
				hello_timer_(this),
				tc_timer_(this),
				mid_timer_(this) {

	// Enable usage of some of the configuration variables from Tcl.
	//
	// Note: Do NOT change the values of these variables in the constructor
	// after binding them! The desired default values should be set in
	// ns-X.XX/tcl/lib/ns-default.tcl instead.
	bind("willingness_", &willingness_);
	bind("hello_ival_", &hello_ival_);
	bind("tc_ival_", &tc_ival_);
	bind("mid_ival_", &mid_ival_);
	bind_bool("use_mac_", &use_mac_);
	
	// Do some initializations
	ra_address_	= id;
	pqt_seq_	= olsr_md_MAXI_SEQ_NUM;
	msg_seq_	= olsr_md_MAXI_SEQ_NUM;
	ansn_		= olsr_md_MAXI_SEQ_NUM;

	paquet_count_ 	= 0;
}

///
/// \brief	This function is called whenever a packet is received. It identifies
///		the type of the received packet and process it accordingly.
///
/// If it is an %olsr_md packet then it is processed. In other case, if it is a data packet
/// then it is forwarded.
///
/// \param	p the received packet.
/// \param	h a handler (not used).
///
void
olsr_md::recv(paquet* p, Handler* h) {
	struct hdm_cmn* ch	= HDM_CMN(p);
	struct hdm_ip* ih	= HDM_IP(p);
	struct olsr_md_sr* srh	= olsr_md_sr(p);   //the head for source routing

//	printf ("reciving...\n");
//	for(;;)
//	if(ifq->prq_length()>10)
//	printf("%d \t ", ifq->prq_length());

	nsaddress_t dest = ih->daddress();
	
	if (ih->saddress() == ra_address()) {
		// If there exists a loop, must drop the packet
		if (ch->num_forwards() > 0) {
			drop(p, DROP_RTR_ROUTE_LOOP);
			return;
		}
		// else if this is a packet I am originating, must add IP header
		else if (ch->num_forwards() == 0){
	//		printf("from the top...No11. %d,%d\n", paquet_count_,ih->saddress());
			ch->size() += IP_HDM_LEN;
		//	ch->error() = 0;
			
			
			//check the flag of routing table. If it is out of date, recompute the routing table
			if(m_rtbl_.get_flag(dest) == true){
				m_rtbl_computation(p);
			}

			//this must be an outgoing paquet, it doesn't have a SR header on it
			srh->init();

			//here we must chose one route from the routing table
			molsr_tbl_t::iterator temp_it = m_rtbl_.lookup(ih->daddress());

			//if there is no route, we must drop the paquet and return
			if(temp_it == (*m_rtbl_.molsr_rt()).end()){
				debug("%f: Node %d can not forward a paquet destined to %d\n",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()),
					olsr_md::node_id(ih->daddress()));
				drop(p, DROP_RTR_NO_ROUTE);
				return;
			}
			
			//now find a route
			for (int j = 0;j<paquet_count_%MAXI_ROUTE;j++)
				temp_it++;

			paquet_count_++;
			//now we must fill the source route
			sr_address* temp_address;
			temp_address = srh->address();
			
			olsr_md_m_rt_entry* m_entry = (*temp_it).second;
			for(int i = 0;i<MAXI_SR_LEN;i++){
				*temp_address = m_entry->address_[i];
				temp_address++;
			}
			
		}
	}
	
	// If it is an olsr_md packet, must process it
	if (ch->ptype() == PQT_OLSR_MD)
		recv_olsr_md(p);
	// Otherwise, must forward the packet (unless TTL has reached zero)
	else {
		ih->ttl_--;
		if (ih->ttl_ == 0) {
			drop(p, DROP_RTR_TTL);
			return;
		}
//		printf("call forward... \n");
//		forward_data(p);
		m_forward_data(p);
	}
}

///
/// \brief Processes an incoming %olsr_md packet following RFC 3626 specification.
/// \param p received packet.
///
void
olsr_md::recv_olsr_md(paquet* p) {
	struct hdm_ip* ih	= HDM_IP(p);
	olsr_md_pqt* op		= PKT_OLSR_MD(p);
	
	// All routing messages are sent from and to port ROUT_PORT,
	// so we check it.
	assert(ih->sport() == ROUT_PORT);
	assert(ih->dport() == ROUT_PORT);
	
//	printf("Receiving olsr_md paquets...\n");
	// If the packet contains no messages must be silently discarded.
	// There could exist a message with an empty body, so the size of
	// the packet would be pqt-hdr-size + msg-hdr-size.
	if (op->pqt_len() < OLSR_MD_PKT_HDM_SIZE + OLSR_MD_MSG_HDM_SIZE) {
		paquet::free(p);
		return;
	}
	
	assert(op->count >= 0 && op->count <= olsr_md_MAXI_MSGS);
	for (int i = 0; i < op->count; i++) {
		olsr_md_msg& msg = op->msg(i);
		
		// If ttl is less than or equal to zero, or
		// the receiver is the same as the originator,
		// the message must be silently dropped
		if (msg.ttl() <= 0 || msg.orig_address() == ra_address())
			continue;
		
		// If the message has been processed it must not be
		// processed again
		bool do_forwarding = true;
		olsr_md_dup_tuple* duplicated = state_.find_dup_tuple(msg.orig_address(), msg.msg_seq_num());
		if (duplicated == NULL) {
			// Process the message according to its type
			if (msg.msg_type() == OLSR_MD_HELLO_MSG)
				process_hello(msg, ra_address(), ih->saddress());
			else if (msg.msg_type() == OLSR_MD_TC_MSG)
				process_tc(msg, ih->saddress());
			else if (msg.msg_type() == OLSR_MD_MID_MSG)
				process_mid(msg, ih->saddress());
			else {
				debug("%f: Node %d can not process olsr_md paquet because does not "
					"implement olsr_md type (%x)\n",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()),
					msg.msg_type());
			}
		}
		else {
			// If the message has been considered for forwarding, it should
			// not be retransmitted again
			for (address_list_t::iterator it = duplicated->intface_list().begin();
				it != duplicated->intface_list().end();
				it++) {
				if (*it == ra_address()) {
					do_forwarding = false;
					break;
				}
			}
		}
			
		if (do_forwarding) {
			// HELLO messages are never forwarded.
			// TC and MID messages are forwarded using the default algorithm.
			// Remaining messages are also forwarded using the default algorithm.
			if (msg.msg_type() != OLSR_MD_HELLO_MSG)
				forward_default(p, msg, duplicated, ra_address());
		}

	}
	
	// After processing all olsr_md messages, we must recompute routing table,for unipath routing
//	rtable_computation();
	
	//for multipath rouitng, instead of recompute the routing table, just set the flag
	m_rtbl_.set_flag(true);
	
	// Release resources
	paquet::free(p);
}

///
/// \brief Computates MPR set of a node following RFC 3626 hints.
///
void
olsr_md::mpr_computation() {
	// MPR computation should be done for each interface. See section 8.3.1
	// (RFC 3626) for details.
	
	state_.clear_mprset();
//	printf("computing mpr\n");
	
	nbset_t N; nb2hopset_t N2;
	// N is the subset of neighbors of the node, which are
	// neighbor "of the interface I"
	for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
		if ((*it)->status() == OLSR_MD_STATUS_SYM) // I think that we need this check
			N.push_back(*it);
	
	// N2 is the set of 2-hop neighbors reachable from "the interface
	// I", excluding:
	// (i)   the nodes only reachable by members of N with willingness WILL_NEVER
	// (ii)  the node performing the computation
	// (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
	//       lnk to this node on some interface.
	for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
		olsr_md_nb2hop_tuple* nb2hop_tuple = *it;
		bool ok = true;
		olsr_md_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_address());
		if (nb_tuple == NULL)
			ok = false;
		else {
			nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_address(), OLSR_MD_WILL_NEVER);
			if (nb_tuple != NULL)
				ok = false;
			else {
				nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_address());
				if (nb_tuple != NULL)
					ok = false;
			}
		}

		if (ok)
			N2.push_back(nb2hop_tuple);
	}
	
	// 1. Start with an MPR set made of all members of N with
	// N_willingness equal to WILL_ALWAYS
	for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
		olsr_md_nb_tuple* nb_tuple = *it;
		if (nb_tuple->willingness() == OLSR_MD_WILL_ALWAYS)
			state_.insert_mpr_address(nb_tuple->nb_main_address());
	}
	
	// 2. Calculate D(y), where y is a member of N, for all nodes in N.
	// We will do this later.
	
	// 3. Add to the MPR set those nodes in N, which are the *only*
	// nodes to provide reachability to a node in N2. Remove the
	// nodes from N2 which are now covered by a node in the MPR set.
	mpr_set_t foundset;
	std::set<nsaddress_t> deleted_address;
	for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
		olsr_md_nb2hop_tuple* nb2hop_tuple1 = *it;
		
		mpr_set_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_address());
		if (pos != foundset.end())
			continue;
		
		bool found = false;
		for (nbset_t::iterator it2 = N.begin(); it2 != N.end(); it2++) {
			if ((*it2)->nb_main_address() == nb2hop_tuple1->nb_main_address()) {
				found = true;
				break;
			}
		}
		if (!found)
			continue;
		
		found = false;
		for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
			olsr_md_nb2hop_tuple* nb2hop_tuple2 = *it2;
			if (nb2hop_tuple1->nb2hop_address() == nb2hop_tuple2->nb2hop_address()) {
				foundset.insert(nb2hop_tuple1->nb2hop_address());
				found = true;
				break;
			}
		}
		if (!found) {
			state_.insert_mpr_address(nb2hop_tuple1->nb_main_address());
			
			for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
				olsr_md_nb2hop_tuple* nb2hop_tuple2 = *it2;
				if (nb2hop_tuple1->nb_main_address() == nb2hop_tuple2->nb_main_address()) {
					deleted_address.insert(nb2hop_tuple2->nb2hop_address());
					it2 = N2.erase(it2);
					it2--;
				}
			}
			it = N2.erase(it);
			it--;
		}
		
		for (std::set<nsaddress_t>::iterator it2 = deleted_address.begin();
			it2 != deleted_address.end();
			it2++) {
			for (nb2hopset_t::iterator it3 = N2.begin();
				it3 != N2.end();
				it3++) {
				if ((*it3)->nb2hop_address() == *it2) {
					it3 = N2.erase(it3);
					it3--;
					// I have to reset the external iterator because it
					// may have been invalidated by the latter deletion
					it = N2.begin();
					it--;
				}
			}
		}
		deleted_address.clear();
	}
	
	// 4. While there exist nodes in N2 which are not covered by at
	// least one node in the MPR set:
	while (N2.begin() != N2.end()) {
		// 4.1. For each node in N, calculate the reachability, i.e., the
		// number of nodes in N2 which are not yet covered by at
		// least one node in the MPR set, and which are reachable
		// through this 1-hop neighbor
		map<int, std::vector<olsr_md_nb_tuple*> > reachability;
		set<int> rs;
		for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
			olsr_md_nb_tuple* nb_tuple = *it;
			int r = 0;
			for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
				olsr_md_nb2hop_tuple* nb2hop_tuple = *it2;
				if (nb_tuple->nb_main_address() == nb2hop_tuple->nb_main_address())
					r++;
			}
			rs.insert(r);
			reachability[r].push_back(nb_tuple);
		}
		
		// 4.2. Select as a MPR the node with highest N_willingness among
		// the nodes in N with non-zero reachability. In case of
		// multiple choice select the node which provides
		// reachability to the maximum number of nodes in N2. In
		// case of multiple nodes providing the same amount of
		// reachability, select the node as MPR whose D(y) is
		// greater. Remove the nodes from N2 which are now covered
		// by a node in the MPR set.
		olsr_md_nb_tuple* max = NULL;
		int maxi_r = 0;
		for (set<int>::iterator it = rs.begin(); it != rs.end(); it++) {
			int r = *it;
			if (r > 0) {
				for (std::vector<olsr_md_nb_tuple*>::iterator it2 = reachability[r].begin();
					it2 != reachability[r].end();
					it2++) {
					olsr_md_nb_tuple* nb_tuple = *it2;
					if (max == NULL || nb_tuple->willingness() > max->willingness()) {
						max = nb_tuple;
						maxi_r = r;
					}
					else if (nb_tuple->willingness() == max->willingness()) {
						if (r > maxi_r) {
							max = nb_tuple;
							maxi_r = r;
						}
						else if (r == maxi_r) {
							if (degree(nb_tuple) > degree(max)) {
								max = nb_tuple;
								maxi_r = r;
							}
						}
					}
				}
			}
		}
		if (max != NULL) {
			state_.insert_mpr_address(max->nb_main_address());
			std::set<nsaddress_t> nb2hop_address;
			for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
				olsr_md_nb2hop_tuple* nb2hop_tuple = *it;
				if (nb2hop_tuple->nb_main_address() == max->nb_main_address()) {
					nb2hop_address.insert(nb2hop_tuple->nb2hop_address());
					it = N2.erase(it);
					it--;
				}
			}
			for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
				olsr_md_nb2hop_tuple* nb2hop_tuple = *it;
				std::set<nsaddress_t>::iterator it2 =
					nb2hop_address.find(nb2hop_tuple->nb2hop_address());
				if (it2 != nb2hop_address.end()) {
					it = N2.erase(it);
					it--;
				}
			}
		}
	}
}

///
/// \brief Creates the routing table of the node following RFC 3626 hints.
///
void
olsr_md::rtable_computation() {
	// 1. All the entries from the routing table are removed.
	rtable_.clear();
	
	// 2. The new routing entries are added starting with the
	// symmetric neighbors (h=1) as the destination nodes.
	for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++) {
		olsr_md_nb_tuple* nb_tuple = *it;
		if (nb_tuple->status() == OLSR_MD_STATUS_SYM) {
			bool nb_main_address = false;
			olsr_md_lnk_tuple* lt = NULL;
			for (lnkset_t::iterator it2 = lnkset().begin(); it2 != lnkset().end(); it2++) {
				olsr_md_lnk_tuple* lnk_tuple = *it2;
				if (get_main_address(lnk_tuple->nb_intface_address()) == nb_tuple->nb_main_address() && lnk_tuple->time() >= CURRENT_TIME) {
					lt = lnk_tuple;
					rtable_.add_entry(lnk_tuple->nb_intface_address(),
							lnk_tuple->nb_intface_address(),
							lnk_tuple->local_intface_address(),
							1);
					if (lnk_tuple->nb_intface_address() == nb_tuple->nb_main_address())
						nb_main_address = true;
				}
			}
			if (!nb_main_address && lt != NULL) {
				rtable_.add_entry(nb_tuple->nb_main_address(),
						lt->nb_intface_address(),
						lt->local_intface_address(),
						1);
			}
		}
	}
	
	// N2 is the set of 2-hop neighbors reachable from this node, excluding:
	// (i)   the nodes only reachable by members of N with willingness WILL_NEVER
	// (ii)  the node performing the computation
	// (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
	//       lnk to this node on some interface.
	for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
		olsr_md_nb2hop_tuple* nb2hop_tuple = *it;
		bool ok = true;
		olsr_md_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_address());
		if (nb_tuple == NULL)
			ok = false;
		else {
			nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_address(), OLSR_MD_WILL_NEVER);
			if (nb_tuple != NULL)
				ok = false;
			else {
				nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_address());
				if (nb_tuple != NULL)
					ok = false;
			}
		}

		// 3. For each node in N2 create a new entry in the routing table
		if (ok) {
			olsr_md_rt_entry* entry = rtable_.lookup(nb2hop_tuple->nb_main_address());
			assert(entry != NULL);
			rtable_.add_entry(nb2hop_tuple->nb2hop_address(),
					entry->next_address(),
					entry->intface_address(),
					2);
		}
	}
	
	for (u_int32_t h = 2; ; h++) {
		bool added = false;
		
		// 4.1. For each topology entry in the topology table, if its
		// T_destination_address does not correspond to R_destination_address of any
		// route entry in the routing table AND its T_last_address
		// corresponds to R_destination_address of a route entry whose R_dist
		// is equal to h, then a new route entry MUST be recorded in
		// the routing table (if it does not already exist)
		for (topologyset_t::iterator it = topologyset().begin();
			it != topologyset().end();
			it++) {
			//printf("computing tc set\n");
			olsr_md_topology_tuple* topology_tuple = *it;
			olsr_md_rt_entry* entry1 = rtable_.lookup(topology_tuple->destination_address());
			olsr_md_rt_entry* entry2 = rtable_.lookup(topology_tuple->last_address());
			if (entry1 == NULL && entry2 != NULL && entry2->dist() == h) {
				rtable_.add_entry(topology_tuple->destination_address(),
						entry2->next_address(),
						entry2->intface_address(),
						h+1);
				added = true;
			}
		}
		
		// 5. For each entry in the multiple interface association base
		// where there exists a routing entry such that:
		//	R_destination_address  == I_main_address  (of the multiple interface association entry)
		// AND there is no routing entry such that:
		//	R_destination_address  == I_intface_address
		// then a route entry is created in the routing table
		for (intfaceassocset_t::iterator it = intfaceassocset().begin();
			it != intfaceassocset().end();
			it++) {
			olsr_md_intface_assoc_tuple* tuple = *it;
			olsr_md_rt_entry* entry1 = rtable_.lookup(tuple->main_address());
			olsr_md_rt_entry* entry2 = rtable_.lookup(tuple->intface_address());
			if (entry1 != NULL && entry2 == NULL) {
				rtable_.add_entry(tuple->intface_address(),
						entry1->next_address(),
						entry1->intface_address(),
						entry1->dist());
				added = true;
			}
		}

		if (!added)
			break;
	}
}

void 
olsr_md::m_rtbl_computation(paquet* p){
		struct hdm_cmn* ch	= HDM_CMN(p);
		struct hdm_ip* ih	= HDM_IP(p);
		struct olsr_md_sr* srh	= olsr_md_sr(p);   //the head for source routing
		
		struct sr_address address_[MAXI_SR_LEN];
		double start_time, end_time;
		int sr_count_ = 0;
		bool type_flag = false;
	
		printf("\n paquet ID:%d",ch->uid_);
			
		map<nsaddress_t,Dijkstra_node*>::iterator n_it,n_it_d;
		
		//destination
		nsaddress_t dest = ih->daddress();
		//source
		nsaddress_t source = ra_address();
		
		//clear the old entries
		m_rtbl_.rm_entry(dest);
	
		//gather the topology info: topology set, nb set, 2nbset
		/*			olsr_md_topology_tuple* topology_tuple = new olsr_md_topology_tuple;
				topology_tuple->destination_address() = address; 
				topology_tuple->last_address() = msg.orig_address();
				topology_tuple->seq()		= tc.ansn();
				topology_tuple->time()		= now + olsr_md::emf_to_seconds(msg.vtime());
				topology_tuple->weight()	= 1;//initialize the weight
				add_topology_tuple(topology_tuple);
		*/
	
		start_time = CURRENT_TIME;
		topologyset_t	n_topologyset_; 
		olsr_md_topology_tuple* tt;
		for (topologyset_t::iterator it = topologyset().begin(); it != topologyset().end(); it++){
			tt = new olsr_md_topology_tuple;
			tt->destination_address() = (*it)->destination_address_;
			tt->last_address() = (*it)->last_address_;
			tt->weight() = 1;	
			if(tt->destination_address()==ra_address()||tt->last_address()==ra_address()){
				delete tt;
				continue;
			}
			n_topologyset_.push_back(tt);
		}
		for (nbset_t::iterator it = nbset().begin();it!=nbset().end();it++){
			tt = new olsr_md_topology_tuple;
			tt->destination_address() = (*it)->nb_main_address_;
			tt->last_address() = ra_address();
			tt->weight() = 1;
			n_topologyset_.push_back(tt);
			//printf("%d, %d|\t", tt->last_address(),tt->destination_address());
		}
		for(nb2hopset_t::iterator it = nb2hopset().begin();it!=nb2hopset().end();it++){
			tt = new olsr_md_topology_tuple;
			tt->destination_address() = (*it)->nb2hop_address_;
			tt->last_address() = (*it)->nb_main_address_;
			tt->weight() = 1;
			if(tt->destination_address()==ra_address()||tt->last_address()==ra_address()){
				delete tt;
				continue;
			}
			n_topologyset_.push_back(tt);
		}
		
		//get the node map
		map<nsaddress_t, Dijkstra_node*> node_map_;
	
		//add the node itself
		pair<map<nsaddress_t,Dijkstra_node*>::iterator,bool> ret;
		Dijkstra_node* node = new Dijkstra_node;
		(*node).address_ = ra_address();
		(*node).weight_ = MAXI_WEIGHT;
		(*node).node_type = 0;
		ret = node_map_.insert(pair<nsaddress_t,Dijkstra_node*>((*node).address_,node));
		
		for (topologyset_t::iterator it = n_topologyset_.begin(); it != n_topologyset_.end(); it++){
			//unvisited_.insert(pair<nsaddress_t,float>((*it)->destination_address_,MAXI_WEIGHT));
			//	unvisited_.insert(pair<nsaddress_t,float>((*it)->last_address_,MAXI_WEIGHT));
			pair<map<nsaddress_t,Dijkstra_node*>::iterator,bool> ret;
			
			Dijkstra_node* node = new Dijkstra_node;
			(*node).address_ = (*it)->last_address_;
			(*node).weight_ = MAXI_WEIGHT;
			(*node).node_type = 0;
					ret = node_map_.insert(pair<nsaddress_t,Dijkstra_node*>((*node).address_,node));
			if (ret.second == false)	
				delete node;
			
			
			node = new Dijkstra_node;
			(*node).address_ = (*it)->destination_address_;
			(*node).weight_ = MAXI_WEIGHT;
			(*node).node_type = 0;
					ret = node_map_.insert(pair<nsaddress_t,Dijkstra_node*>((*node).address_,node));
			if(ret.second == false)
				delete node;
	
			(**it).weight_ = 1;
			//node_set_.insert(node);
		}
	/*
		//for test print the node set
		int test0 = 0;
		for(n_it = node_map_.begin();n_it != node_map_.end();n_it++){
			printf("%d\t",(*n_it).first);
			//test0++;
		}*/
		printf("\n");
		/*
		//	0 for T (temporary) type, 1 for P type
		//initialize set the source to P type
		n_it = node_map_.find(source);
		(*(*n_it).second).node_type = P_TYPE;
		(*(*n_it).second).weight_ = 0;
		*/
		Dijkstra_node *node_n, *node_t;
		
		for (int i = 0;i<MAXI_ROUTE;i++){
			n_it = node_map_.find(source);
			(*(*n_it).second).node_type = P_TYPE;
			(*(*n_it).second).weight_ = 0;
			for(;;){
				//1. renew all the weight_ of T type nodes
				
				for(topologyset_t::iterator it = n_topologyset_.begin(); it != n_topologyset_.end(); it++){
					for(n_it = node_map_.begin();n_it != node_map_.end();n_it++){
				/*		Dijkstra_node node_s, node_d;
						node_s = *(*n_it).second;
						//this part still need to be modified for uni-path protocol
						if((node_s.node_type == P_TYPE) && ((node_s.address_ == (*it)->last_address_)||(node_s.address_ == (*it)->destination_address_))){
							if (node_s.address_ == (*it)->last_address_)
								n_it_d = node_map_.find((*it)->destination_address_);
							else
								n_it_d = node_map_.find((*it)->last_address_);
							
							node_d = *(*n_it_d).second;
							if (node_s.weight_ + (*it)->weight_ < node_d.weight_){
								node_d.weight_ = node_s.weight_ + (*it)->weight_;
								node_d.pre_address_ = node_s.address_;
							}
							
						}*/
						Dijkstra_node *node_s, *node_d;
						node_s = (*n_it).second;
						//this part still need to be modified for uni-direction protocol
						if(((*node_s).node_type == P_TYPE) && (((*node_s).address_ == (*it)->last_address_)||((*node_s).address_ == (*it)->destination_address_))){
							if ((*node_s).address_ == (*it)->last_address_)
								n_it_d = node_map_.find((*it)->destination_address_);
							else
								n_it_d = node_map_.find((*it)->last_address_);
							
							node_d = (*n_it_d).second;
							if ((*node_s).weight_ + (*it)->weight_ < (*node_d).weight_){
								(*node_d).weight_ = (*node_s).weight_ + (*it)->weight_;
								(*node_d).pre_address_ = (*node_s).address_;
							}
						}
						
					}
				}
				
				//2. find the T type node with the min weight_, and set it to P type
				//Dijkstra_node node_n, *node_t;
				//n_it = node_map_.begin();
				//node_t = (*n_it).second;
				int counter = 0;
				node_t = (*node_map_.begin()).second; //intitilize
				for(n_it = node_map_.begin();n_it != node_map_.end(); n_it++){
					node_n = (*n_it).second;
					if((*node_n).node_type == T_TYPE){
						if (counter == 0) //the first rotation
							node_t = node_n;
						if ((*node_t).weight_ > (*node_n).weight_){
							node_t = node_n;
						}
						counter++;
					}
				}
	
				//the weight is MAX, no route
				if((*node_t).weight_ >= MAXI_WEIGHT){
					printf("\nthere is no route found from %d to %d!!", source,dest);
					//m_rtbl_.set_flag(dest, false);
					type_flag = false;
					return;
					break;
				}
				
				//set to P_TYPE
				(*node_t).node_type = P_TYPE;
	/*
				//now renew the weight of the topology tuple for multipath algorithm
				for(topologyset_t::iterator it = topologyset().begin(); it != topologyset().end(); it++){
					if((*it)->last_address_ == (*node_t).address_){
						(*it)->weight_ = tuple_weight((*it)->weight_);
						printf("%f %d-> %d\n",(*it)->weight_, (*node_t).address_, (*it)->destination_address());
					}
				}*/
				
		
				
				//3. found the route to the destination
				if((*node_t).address_ == dest){
					printf("\nI got one!");
					type_flag = true;
					break;
				}
				
				type_flag = false;
				for(n_it = node_map_.begin();n_it != node_map_.end(); n_it++){
					node_n = (*n_it).second;
					if((*node_n).node_type == T_TYPE)
						type_flag = true;
					//(*node_n).node_type = T_TYPE;
					//(*node_n).weight_ = 0;
				}
				if(type_flag == false){
					printf("\nthere is no route found from %d to %d!!", source,dest);
					
					break;
				}
				
			}
	
			if (type_flag == false)
				break;
			//construct the routing entry
			olsr_md_m_rt_entry* m_rt_entry = new olsr_md_m_rt_entry;
			std::vector<sr_address> address_stack;
			//while((*node_t).address_ != source)
			do	{
				sr_address* sr_ = new sr_address;
				(*sr_).address = (*node_t).address_;
				address_stack.push_back(*sr_);
	
				//now renew the weight of the topology tuple for multipath algorithm
				for(topologyset_t::iterator it = n_topologyset_.begin(); it != n_topologyset_.end(); it++){
					if((*it)->last_address_ == (*node_t).address_){
					//	printf("%f %d-> %d\t",(*it)->weight_, (*node_t).address_, (*it)->destination_address());
						(*it)->weight_ = tuple_weight((*it)->weight_);
						//(*it)->weight_ = (*it)->weight_*2;
			
					}
				}
				
				node_t = node_map_.find((*node_t).pre_address_)->second;
			} while((*node_t).address_ != source);
	
				/*		for(topologyset_t::iterator tit = topologyset().begin(); tit != topologyset().end(); tit++){
					printf("%d--->%d, weight:%f\n", (*tit)->last_address_, (*tit)->destination_address_, (*tit)->weight_);
				}*/
			//add itself
			sr_address* sr_ = new sr_address;
			(*sr_).address = ra_address();
			address_stack.push_back(*sr_);
			
			int i = 0;
			printf("from %d to %d:\t",ra_address(),dest);
			while(address_stack.empty() == false){
				(*m_rt_entry).address_[i].address = address_stack.back().address;
				printf("%d->\t",address_stack.back().address);
				address_stack.pop_back();
				i++;
			}
			
			//add the entry to routing table
			m_rtbl_.add_entry(m_rt_entry,dest);
	
			
	
			//reset all the nodes for next rotation
			for(n_it = node_map_.begin();n_it != node_map_.end(); n_it++){
					node_n = (*n_it).second;
				(*node_n).node_type = T_TYPE;
				(*node_n).weight_ = MAXI_WEIGHT;
			}
		}
		//finish the computation, set the flag
		m_rtbl_.set_flag(dest, false);
		end_time = CURRENT_TIME; 
	
	//	delete *n_topologyset_;

	
}
///
/// \brief Processes a HELLO message following RFC 3626 specification.
///
/// Link sensing and population of the Neighbor Set, 2-hop Neighbor Set and MPR
/// Selector Set are performed.
///
/// \param msg the %olsr_md message which contains the HELLO message.
/// \param receiver_intface the address of the interface where the message was received from.
/// \param sender_intface the address of the interface where the message was sent from.
///
void
olsr_md::process_hello(olsr_md_msg& msg, nsaddress_t receiver_intface, nsaddress_t sender_intface) {
	//printf("processing hello...\n");
	assert(msg.msg_type() == OLSR_MD_HELLO_MSG);

        lnk_sensing(msg, receiver_intface, sender_intface);
	populate_nbset(msg);
	populate_nb2hopset(msg);
	mpr_computation();
	populate_mprselset(msg);
}

///
/// \brief Processes a TC message following RFC 3626 specification.
///
/// The Topology Set is updated (if needed) with the information of
/// the received TC message.
///
/// \param msg the %olsr_md message which contains the TC message.
/// \param sender_intface the address of the interface where the message was sent from.
///
void
olsr_md::process_tc(olsr_md_msg& msg, nsaddress_t sender_intface) {
	assert(msg.msg_type() == OLSR_MD_TC_MSG);
	double now	= CURRENT_TIME;
	olsr_md_tc& tc	= msg.tc();
	
//	printf("processing tc...\n");
	
	// 1. If the sender interface of this message is not in the symmetric
	// 1-hop neighborhood of this node, the message MUST be discarded.
	olsr_md_lnk_tuple* lnk_tuple = state_.find_sym_lnk_tuple(sender_intface, now);
	if (lnk_tuple == NULL)
		return;
	
	// 2. If there exist some tuple in the topology set where:
	// 	T_last_address == originator address AND
	// 	T_seq       >  ANSN,
	// then further processing of this TC message MUST NOT be
	// performed.
	olsr_md_topology_tuple* topology_tuple =
		state_.find_newer_topology_tuple(msg.orig_address(), tc.ansn());
	if (topology_tuple != NULL)
		return;
	
	// 3. All tuples in the topology set where:
	//	T_last_address == originator address AND
	//	T_seq       <  ANSN
	// MUST be removed from the topology set.
	state_.erase_older_topology_tuples(msg.orig_address(), tc.ansn());

	// 4. For each of the advertised neighbor main address received in
	// the TC message:
	for (int i = 0; i < tc.count; i++) {
		assert(i >= 0 && i < OLSR_MD_MAXI_ADDRS);
		nsaddress_t address = tc.nb_main_address(i);
		// 4.1. If there exist some tuple in the topology set where:
		// 	T_destination_address == advertised neighbor main address, AND
		// 	T_last_address == originator address,
		// then the holding time of that tuple MUST be set to:
		// 	T_time      =  current time + validity time.
		olsr_md_topology_tuple* topology_tuple =
			state_.find_topology_tuple(address, msg.orig_address());
		if (topology_tuple != NULL)
			topology_tuple->time() = now + olsr_md::emf_to_seconds(msg.vtime());
		// 4.2. Otherwise, a new tuple MUST be recorded in the topology
		// set where:
		//	T_destination_address = advertised neighbor main address,
		//	T_last_address = originator address,
		//	T_seq       = ANSN,
		//	T_time      = current time + validity time.
		else {
			olsr_md_topology_tuple* topology_tuple = new olsr_md_topology_tuple;
			topology_tuple->destination_address()	= address; 
			topology_tuple->last_address()	= msg.orig_address();
			topology_tuple->seq()		= tc.ansn();
			topology_tuple->time()		= now + olsr_md::emf_to_seconds(msg.vtime());
			topology_tuple->weight()	= 1;//initialize the weight
			add_topology_tuple(topology_tuple);
			// Schedules topology tuple deletion
			olsr_md_TopologyTupleTimer* topology_timer =
				new olsr_md_TopologyTupleTimer(this, topology_tuple);
			topology_timer->resched(DELAY(topology_tuple->time()));
		}
	}
}

///
/// \brief Processes a MID message following RFC 3626 specification.
///
/// The Interface Association Set is updated (if needed) with the information
/// of the received MID message.
///
/// \param msg the %olsr_md message which contains the MID message.
/// \param sender_intface the address of the interface where the message was sent from.
///
void
olsr_md::process_mid(olsr_md_msg& msg, nsaddress_t sender_intface) {
	assert(msg.msg_type() == OLSR_MD_MID_MSG);
	double now	= CURRENT_TIME;
	olsr_md_mid& mid	= msg.mid();
	
	// 1. If the sender interface of this message is not in the symmetric
	// 1-hop neighborhood of this node, the message MUST be discarded.
	olsr_md_lnk_tuple* lnk_tuple = state_.find_sym_lnk_tuple(sender_intface, now);
	if (lnk_tuple == NULL)
		return;
	
	// 2. For each interface address listed in the MID message
	for (int i = 0; i < mid.count; i++) {
		bool updated = false;
		for (intfaceassocset_t::iterator it = intfaceassocset().begin();
			it != intfaceassocset().end();
			it++) {
			olsr_md_intface_assoc_tuple* tuple = *it;
			if (tuple->intface_address() == mid.intface_address(i)
				&& tuple->main_address() == msg.orig_address()) {
				tuple->time()	= now + olsr_md::emf_to_seconds(msg.vtime());
				updated		= true;
			}			
		}
		if (!updated) {
			olsr_md_intface_assoc_tuple* tuple	= new olsr_md_intface_assoc_tuple;
			tuple->intface_address()		= msg.mid().intface_address(i);
			tuple->main_address()		= msg.orig_address();
			tuple->time()			= now + olsr_md::emf_to_seconds(msg.vtime());
			add_intfaceassoc_tuple(tuple);
			// Schedules intface association tuple deletion
			olsr_md_IfaceAssocTupleTimer* intfaceassoc_timer =
				new olsr_md_IfaceAssocTupleTimer(this, tuple);
			intfaceassoc_timer->resched(DELAY(tuple->time()));
		}
	}
}

///
/// \brief olsr_md's default forwarding algorithm.
///
/// See RFC 3626 for details.
///
/// \param p the %olsr_md packet which has been received.
/// \param msg the %olsr_md message which must be forwarded.
/// \param dup_tuple NULL if the message has never been considered for forwarding,
/// or a duplicate tuple in other case.
/// \param local_intface the address of the interface where the message was received from.
///
void
olsr_md::forward_default(paquet* p, olsr_md_msg& msg, olsr_md_dup_tuple* dup_tuple, nsaddress_t local_intface) {
	double now		= CURRENT_TIME;
	struct hdm_ip* ih	= HDM_IP(p);
	
	// If the sender interface address is not in the symmetric
	// 1-hop neighborhood the message must not be forwarded
	olsr_md_lnk_tuple* lnk_tuple = state_.find_sym_lnk_tuple(ih->saddress(), now);
	if (lnk_tuple == NULL)
		return;

	// If the message has already been considered for forwarding,
	// it must not be retransmitted again
	if (dup_tuple != NULL && dup_tuple->retransmitted()) {
		debug("%f: Node %d does not forward a message received"
			" from %d because it is duplicated\n",
			CURRENT_TIME,
			olsr_md::node_id(ra_address()),
			olsr_md::node_id(dup_tuple->address()));
		return;
	}
	
	// If the sender interface address is an interface address
	// of a MPR selector of this node and ttl is greater than 1,
	// the message must be retransmitted
	bool retransmitted = false;
	if (msg.ttl() > 1) {
		olsr_md_mprsel_tuple* mprsel_tuple =
			state_.find_mprsel_tuple(get_main_address(ih->saddress()));
		if (mprsel_tuple != NULL) {
			olsr_md_msg& new_msg = msg;
			new_msg.ttl()--;
			new_msg.hop_count()++;
			// We have to introduce a random delay to avoid
			// synchronization with neighbors.
			enque_msg(new_msg, JITTER);
			retransmitted = true;
		}
	}
	
	// Update duplicate tuple...
	if (dup_tuple != NULL) {
		dup_tuple->time()		= now + olsr_md_DUP_HOLD_TIME;
		dup_tuple->retransmitted()	= retransmitted;
		dup_tuple->intface_list().push_back(local_intface);
	}
	// ...or create a new one
	else {
		olsr_md_dup_tuple* new_dup = new olsr_md_dup_tuple;
		new_dup->address()			= msg.orig_address();
		new_dup->seq_num()		= msg.msg_seq_num();
		new_dup->time()			= now + OLSR_MD_DUP_HOLD_TIME;
		new_dup->retransmitted()	= retransmitted;
		new_dup->intface_list().push_back(local_intface);
		add_dup_tuple(new_dup);
		// Schedules dup tuple deletion
		olsr_md_DupTupleTimer* dup_timer =
			new olsr_md_DupTupleTimer(this, new_dup);
		dup_timer->resched(DELAY(new_dup->time()));
	}
}

///
/// \brief Forwards a data packet to the appropiate next hop indicated by the routing table.
///
/// \param p the packet which must be forwarded.
///
void
olsr_md::forward_data(paquet* p) {
	struct hdm_cmn* ch	= HDM_CMN(p);
	struct hdm_ip* ih	= HDM_IP(p);
	printf("forwarding data..\n");

	if (ch->direction() == hdm_cmn::UP &&
		((u_int32_t)ih->daddress() == IP_BROADCAST || ih->daddress() == ra_address())) {
		dmux_->recv(p, 0);
		return;
	}
	else {
		ch->direction()	= hdm_cmn::DOWN;
		ch->address_type()	= NS_AF_INET;
		if ((u_int32_t)ih->daddress() == IP_BROADCAST)
			ch->next_hop()	= IP_BROADCAST;
		else {
			olsr_md_rt_entry* entry = rtable_.lookup(ih->daddress());
			if (entry == NULL) {
				debug("%f: Node %d can not forward a paquet destined to %d\n",
					CURRENT_TIME,
					olsr_md::node_id(ra_address()),
					olsr_md::node_id(ih->daddress()));
				drop(p, DROP_RTR_NO_ROUTE);
				return;
			}
			else {
				entry = rtable_.find_send_entry(entry);
				assert(entry != NULL);
				ch->next_hop() = entry->next_address();
				if (USE_MAC) {
					ch->xmint_failure_	= olsr_md_mac_failed_callback;
					ch->xmint_failure_data_	= (void*)this;
				}
			}
		}

		Scheduler::instance().schedule(target_, p, 0.0);
		//target_->recv(p);
	}
}

///
///\brief forward a data paquet for multipath routing
///
void
olsr_md::m_forward_data(paquet* p){
	struct hdm_cmn* ch	= HDM_CMN(p);
	struct hdm_ip* ih	= HDM_IP(p);
	struct olsr_md_sr* srh = olsr_md_sr(p);

	struct sr_address* local_sr;
	int i;
	bool flag = false;
//	ch->uid_ = 0;

	if (ch->direction() == hdm_cmn::UP &&
		((u_int32_t)ih->daddress() == IP_BROADCAST || ih->daddress() == ra_address())) {
		dmux_->recv(p, 0);
		return;
	}
	else{
		ch->direction()	= hdm_cmn::DOWN;
		ch->address_type()	= NS_AF_INET;
		if ((u_int32_t)ih->daddress() == IP_BROADCAST)
			ch->next_hop()	= IP_BROADCAST;
		else{
  			printf("\nQueue_length:\t%f\t%d\t%d\t%d", CURRENT_TIME, ch->uid(),ra_address(), ifq->prq_length());			
			local_sr = srh->address();
			for(i = 0;i<MAXI_SR_LEN;i++,local_sr++){
				if((*local_sr).address== ra_address()){
					local_sr++;
					break;
				}
			}	
			ch->next_hop() = (*local_sr).address;

			//check the neighbor
			for(nbset_t::iterator it = nbset().begin();it!=nbset().end();it++){
				if((*it)->nb_main_address_==(*local_sr).address){
					flag = true;
					break;
				}
			}
			if (flag == false){
				printf("\n%d has no neighbor %d from %d to %d.",ra_address(),ch->next_hop(),  ih->saddress(),ih->daddress()); 
		//		ch->xmint_reason_ = 1;

		
				resend_data(p);
				return;
			}

			
	//		printf("\nforward from %d to %d",ra_address(),ch->next_hop());
			if (USE_MAC) {
				ch->xmint_failure_	= olsr_md_mac_failed_callback;
				ch->xmint_failure_data_	= (void*)this;
			}
			//srh.address_;
			
		}
	}
	

	Scheduler::instance().schedule(target_, p, 0.0);
	
}

void olsr_md::resend_data(paquet* p){
	struct hdm_cmn* ch	= HDM_CMN(p);
	struct hdm_ip* ih	= HDM_IP(p);
	struct olsr_md_sr* srh	= olsr_md_sr(p);   //the head for source routing

	nsaddress_t source = ra_address();
	
	nsaddress_t dest = ih->daddress();
	//change the source addressess
	ih->saddress() = source;
	printf("\nrecomputing...");
	if(m_rtbl_.get_flag(dest) == true){
		m_rtbl_computation(p);
	}

	//here we must chose one route from the routing table
	molsr_tbl_t::iterator temp_it = m_rtbl_.lookup(ih->daddress());

	//if there is no route, we must drop the paquet and return
	if(temp_it == (*m_rtbl_.molsr_rt()).end()){
		debug("%f: Node %d can not forward a paquet destined to %d\n",
			CURRENT_TIME,
			olsr_md::node_id(ra_address()),
			olsr_md::node_id(ih->daddress()));
		drop(p, DROP_RTR_NO_ROUTE);
		return;
	}
			
	//now find a route
	for (int j = 0;j<paquet_count_%MAXI_ROUTE;j++)
		temp_it++;

		paquet_count_++;
		//now we must fill the source route
		sr_address* temp_address;
		temp_address = srh->address();
			
		olsr_md_m_rt_entry* m_entry = (*temp_it).second;
		for(int i = 0;i<MAXI_SR_LEN;i++){
			*temp_address = m_entry->address_[i];
			temp_address++;
		}	

	ih->ttl_--;
	if (ih->ttl_ == 0) {
		drop(p, DROP_RTR_TTL);
		return;
	}
//		printf("call forward... \n");
//		forward_data(p);
	m_forward_data(p);
}

///
/// \brief Enques an %olsr_md message which will be sent with a delay of (0, delay].
///
/// This buffering system is used in order to piggyback several %olsr_md messages in
/// a same %olsr_md packet.
///
/// \param msg the %olsr_md message which must be sent.
/// \param delay maximum delay the %olsr_md message is going to be buffered.
///
void
olsr_md::enque_msg(olsr_md_msg& msg, double delay) {
	assert(delay >= 0);
	
	msgs_.push_back(msg);
	olsr_md_MsgTimer* timer = new olsr_md_MsgTimer(this);
	timer->resched(delay);
}

///
/// \brief Creates as many %olsr_md packet as needed in order to send all buffered
/// %olsr_md messages.
///
/// Maximum number of messages which can be contained in an %olsr_md packet is
/// dictated by olsr_md_MAXI_MSGS constant.
///
void
olsr_md::send_pqt() {
	int num_msgs = msgs_.size();
	if (num_msgs == 0)
		return;
//	printf("sending pqt...\n");
	// Calculates the number of needed packet
	int num_pqts = (num_msgs%OLSR_MD_MAXI_MSGS == 0) ? num_msgs/OLSR_MD_MAXI_MSGS :
		(num_msgs/OLSR_MD_MAXI_MSGS + 1);
	
	for (int i = 0; i < num_pqts; i++) {
		paquet* p		= allocpqt();
		struct hdm_cmn* ch	= HDM_CMN(p);
		struct hdm_ip* ih	= HDM_IP(p);
		olsr_md_pqt* op		= PKT_OLSR_MD(p);
		
		op->pqt_len()		= OLSR_MD_PKT_HDM_SIZE;
		op->pqt_seq_num()	= pqt_seq();
	
		int j = 0;
		for (std::vector<olsr_md_msg>::iterator it = msgs_.begin(); it != msgs_.end(); it++) {
			if (j == OLSR_MD_MAXI_MSGS)
				break;
			
			op->pqt_body_[j++]	= *it;
			op->count		= j;
			op->pqt_len()		+= (*it).size();
			
			it = msgs_.erase(it);
			it--;
		}
	
		ch->ptype()		= PQT_OLSR_MD;
		ch->direction()		= hdm_cmn::DOWN;
		ch->size()		= IP_HDM_LEN + UDP_HDM_LEN + op->pqt_len();
		ch->error()		= 0;
		ch->next_hop()		= IP_BROADCAST;
		ch->address_type()		= NS_AF_INET;
		if (USE_MAC) {
			ch->xmint_failure_	= olsr_md_mac_failed_callback;
			ch->xmint_failure_data_	= (void*)this;
		}

		ih->saddress()	= ra_address();
		ih->daddress()	= IP_BROADCAST;
		ih->sport()	= ROUT_PORT;
		ih->dport()	= ROUT_PORT;
		ih->ttl()	= IP_DEF_TTL;
		
		Scheduler::instance().schedule(target_, p, 0.0);
	}
}

///
/// \brief Creates a new %olsr_md HELLO message which is buffered to be sent later on.
///
void
olsr_md::send_hello() {
	olsr_md_msg msg;
//	printf("sending hello...\n");
	double now		= CURRENT_TIME;
	msg.msg_type()		= OLSR_MD_HELLO_MSG;
	msg.vtime()		= olsr_md::seconds_to_emf(OLSR_MD_NEIGHB_HOLD_TIME);
	msg.orig_address()		= ra_address();
	msg.ttl()		= 1;
	msg.hop_count()		= 0;
	msg.msg_seq_num()	= msg_seq();
	
	msg.hello().reserved()		= 0;
	msg.hello().htime()		= olsr_md::seconds_to_emf(hello_ival());
	msg.hello().willingness()	= willingness();
	msg.hello().count		= 0;
	
	map<u_int8_t, int> lnkcodes_count;
	for (lnkset_t::iterator it = lnkset().begin(); it != lnkset().end(); it++) {
		olsr_md_lnk_tuple* lnk_tuple = *it;
		if (lnk_tuple->local_intface_address() == ra_address() && lnk_tuple->time() >= now) {
			u_int8_t lnk_type, nb_type, lnk_code;
			
			// Establishes lnk type
			if (USE_MAC&& lnk_tuple->lost_time() >= now)
				lnk_type = OLSR_MD_LOST_LNK;
			else if (lnk_tuple->sym_time() >= now)
				lnk_type = OLSR_MD_SYM_LNK;
			else if (lnk_tuple->asym_time() >= now)
				lnk_type = OLSR_MD_ASYM_LNK;
			else
				lnk_type = OLSR_MD_LOST_LNK;
			// Establishes neighbor type.
			if (state_.find_mpr_address(get_main_address(lnk_tuple->nb_intface_address())))
				nb_type = OLSR_MD_MPR_NEIGH;
			else {
				bool ok = false;
				for (nbset_t::iterator nb_it = nbset().begin();
					nb_it != nbset().end();
					nb_it++) {
					olsr_md_nb_tuple* nb_tuple = *nb_it;
					if (nb_tuple->nb_main_address() == lnk_tuple->nb_intface_address()) {
						if (nb_tuple->status() == OLSR_MD_STATUS_SYM)
							nb_type = OLSR_MD_SYM_NEIGH;
						else if (nb_tuple->status() == OLSR_MD_STATUS_NOT_SYM)
							nb_type = OLSR_MD_NOT_NEIGH;
						else {
							fprintf(stderr, "There is a neighbor tuple"
								" with an unknown status!\n");
							exit(1);
						}
						ok = true;
						break;
					}
				}
				if (!ok) {
					fprintf(stderr, "Link tuple has no corresponding"
						" Neighbor tuple\n");
					//exit(1);

				}
			}

			int count = msg.hello().count;
			lnk_code = (lnk_type & 0x03) | ((nb_type << 2) & 0x0f);
			map<u_int8_t, int>::iterator pos = lnkcodes_count.find(lnk_code);
			if (pos == lnkcodes_count.end()) {
				lnkcodes_count[lnk_code] = count;
				assert(count >= 0 && count < OLSR_MD_MAXI_HELLOS);
				msg.hello().hello_msg(count).count = 0;
				msg.hello().hello_msg(count).lnk_code() = lnk_code;
				msg.hello().hello_msg(count).reserved() = 0;
				msg.hello().count++;
			}
			else
				count = (*pos).second;
			
			int i = msg.hello().hello_msg(count).count;
			assert(count >= 0 && count < OLSR_MD_MAXI_HELLOS);
			assert(i >= 0 && i < OLSR_MD_MAXI_ADDRS);
			
			msg.hello().hello_msg(count).nb_intface_address(i) =
				lnk_tuple->nb_intface_address();
			msg.hello().hello_msg(count).count++;
			msg.hello().hello_msg(count).lnk_msg_size() =
				msg.hello().hello_msg(count).size();
		}
	}
	
	msg.msg_size() = msg.size();
	
	enque_msg(msg, JITTER);
}

///
/// \brief Creates a new %olsr_md TC message which is buffered to be sent later on.
///
void
olsr_md::send_tc() {
//	printf("sending tc............................................\n");
	olsr_md_msg msg;
	msg.msg_type()		= OLSR_MD_TC_MSG;
	msg.vtime()		= olsr_md::seconds_to_emf(OLSR_MD_TOP_HOLD_TIME);
	msg.orig_address()		= ra_address();
	msg.ttl()		= 255;
	msg.hop_count()		= 0;
	msg.msg_seq_num()	= msg_seq();
	
	msg.tc().ansn()		= ansn_;
	msg.tc().reserved()	= 0; 
	msg.tc().count		= 0;
	
	//the original for OLSR 
	//add the lnk to mprselecter to tc message
	for (mpr_selset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++) {
		olsr_md_mprsel_tuple* mprsel_tuple = *it;
		int count = msg.tc().count;

		assert(count >= 0 && count < OLSR_MD_MAXI_ADDRS);
		msg.tc().nb_main_address(count) = mprsel_tuple->main_address();
		msg.tc().count++;
	}
	
/*
	//midified for olsr_md
	//add the lnk to all the neighbors to tc message
	for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++){
		olsr_md_nb_tuple* nb_tuple = *it;
		int count = msg.tc().count;
		assert (count >= 0 && count < OLSR_MD_MAXI_ADDRS);
		msg.tc().nb_main_address(count) = nb_tuple->nb_main_address();
		msg.tc().count ++;
	}
*/
	msg.msg_size()		= msg.size();
	
	enque_msg(msg, JITTER);
}

///
/// \brief Creates a new %olsr_md MID message which is buffered to be sent later on.
/// \warning This message is never invoked because there is no support for multiple interfaces.
///
void
olsr_md::send_mid() {
	olsr_md_msg msg;
	msg.msg_type()		= OLSR_MD_MID_MSG;
	msg.vtime()		= olsr_md::seconds_to_emf(OLSR_MD_MID_HOLD_TIME);
	msg.orig_address()		= ra_address();
	msg.ttl()		= 255;
	msg.hop_count()		= 0;
	msg.msg_seq_num()	= msg_seq();
	
	msg.mid().count		= 0;
	//foreach intface in this_node do
	//	msg.mid().intface_address(i) = intface
	//	msg.mid().count++
	//done
	
	msg.msg_size()		= msg.size();
	
	enque_msg(msg, JITTER);
}

///
/// \brief	Updates Link Set according to a new received HELLO message (following RFC 3626
///		specification). Neighbor Set is also updated if needed.
///
/// \param msg the olsr_md message which contains the HELLO message.
/// \param receiver_intface the address of the interface where the message was received from.
/// \param sender_intface the address of the interface where the message was sent from.
///
void
olsr_md::lnk_sensing(olsr_md_msg& msg, nsaddress_t receiver_intface, nsaddress_t sender_intface) {
	olsr_md_hello& hello	= msg.hello();
	double now		= CURRENT_TIME;
	bool updated		= false;
	bool created		= false;
	
	olsr_md_lnk_tuple* lnk_tuple = state_.find_lnk_tuple(sender_intface);
	if (lnk_tuple == NULL) {
		// We have to create a new tuple
		lnk_tuple = new olsr_md_lnk_tuple;
		lnk_tuple->nb_intface_address()	= sender_intface;
		lnk_tuple->local_intface_address()	= receiver_intface;
		lnk_tuple->sym_time()		= now - 1;
		lnk_tuple->lost_time()		= 0.0;
		lnk_tuple->time()		= now + olsr_md::emf_to_seconds(msg.vtime());
		add_lnk_tuple(lnk_tuple, hello.willingness());
		created = true;
	}
	else
		updated = true;
	
 	lnk_tuple->asym_time() = now + olsr_md::emf_to_seconds(msg.vtime());
	assert(hello.count >= 0 && hello.count <= OLSR_MD_MAXI_HELLOS);
	for (int i = 0; i < hello.count; i++) {
		olsr_md_hello_msg& hello_msg = hello.hello_msg(i);
		int lt = hello_msg.lnk_code() & 0x03;
		int nt = hello_msg.lnk_code() >> 2;
		
		// We must not process invalid advertised lnks
		if ((lt == OLSR_MD_SYM_LNK && nt == OLSR_MD_NOT_NEIGH) ||
			(nt != OLSR_MD_SYM_NEIGH && nt != OLSR_MD_MPR_NEIGH
			&& nt != OLSR_MD_NOT_NEIGH))
			continue;
		
		assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MD_MAXI_ADDRS);
		for (int j = 0; j < hello_msg.count; j++) {
			if (hello_msg.nb_intface_address(j) == receiver_intface) {
				if (lt == OLSR_MD_LOST_LNK) {
					lnk_tuple->sym_time() = now - 1;
					updated = true;
				}
				else if (lt == OLSR_MD_SYM_LNK || lt == OLSR_MD_ASYM_LNK) {
					lnk_tuple->sym_time()	=
						now + olsr_md::emf_to_seconds(msg.vtime());
					lnk_tuple->time()	=
						lnk_tuple->sym_time() + OLSR_MD_NEIGHB_HOLD_TIME;
					lnk_tuple->lost_time()	= 0.0;
					updated = true;
				}
				break;
			}
		}
		
	}
	lnk_tuple->time() = MAX(lnk_tuple->time(), lnk_tuple->asym_time());
	
	if (updated)
		updated_lnk_tuple(lnk_tuple);
	
	// Schedules lnk tuple deletion
	if (created && lnk_tuple != NULL) {
		olsr_md_LinkTupleTimer* lnk_timer =
			new olsr_md_LinkTupleTimer(this, lnk_tuple);
		lnk_timer->resched(DELAY(MIN(lnk_tuple->time(), lnk_tuple->sym_time())));
	}
}

///
/// \brief	Updates the Neighbor Set according to the information contained in a new received
///		HELLO message (following RFC 3626).
///
/// \param msg the %olsr_md message which contains the HELLO message.
///
void
olsr_md::populate_nbset(olsr_md_msg& msg) {
	olsr_md_hello& hello = msg.hello();
	
	olsr_md_nb_tuple* nb_tuple = state_.find_nb_tuple(msg.orig_address());
	if (nb_tuple != NULL)
		nb_tuple->willingness() = hello.willingness();
	else
		{
			olsr_md_nb_tuple* nb_tuple		= new olsr_md_nb_tuple;
			nb_tuple->nb_main_address()	= msg.orig_address();
			nb_tuple->willingness()		= hello.willingness();
			//if (tuple->sym_time() >= now)
				nb_tuple->status() = OLSR_MD_STATUS_SYM;
			//else
		//		nb_tuple->status() = OLSR_MD_STATUS_NOT_SYM;
			add_nb_tuple(nb_tuple);			
		//	printf("\nhahaha");
			//getchar();
			
		}
}

///
/// \brief	Updates the 2-hop Neighbor Set according to the information contained in a new
///		received HELLO message (following RFC 3626).
///
/// \param msg the %olsr_md message which contains the HELLO message.
///
void
olsr_md::populate_nb2hopset(olsr_md_msg& msg) {
	double now		= CURRENT_TIME;
	olsr_md_hello& hello	= msg.hello();
	
	for (lnkset_t::iterator it_lt = lnkset().begin(); it_lt != lnkset().end(); it_lt++) {
		olsr_md_lnk_tuple* lnk_tuple = *it_lt;
		if (get_main_address(lnk_tuple->nb_intface_address()) == msg.orig_address()) {
			if (lnk_tuple->sym_time() >= now) {
				assert(hello.count >= 0 && hello.count <= OLSR_MD_MAXI_HELLOS);
				for (int i = 0; i < hello.count; i++) {
					olsr_md_hello_msg& hello_msg = hello.hello_msg(i);
					int nt = hello_msg.lnk_code() >> 2;
					assert(hello_msg.count >= 0 &&
						hello_msg.count <= olsr_md_MAXI_ADDRS);
					
					for (int j = 0; j < hello_msg.count; j++) {
						nsaddress_t nb2hop_address = hello_msg.nb_intface_address(j);
						if (nt == OLSR_MD_SYM_NEIGH || nt == OLSR_MD_MPR_NEIGH) {
							// if the main address of the 2-hop
							// neighbor address = main address of
							// the receiving node: silently
							// discard the 2-hop neighbor address
							if (nb2hop_address != ra_address()) {
								// Otherwise, a 2-hop tuple is created
								olsr_md_nb2hop_tuple* nb2hop_tuple =
									state_.find_nb2hop_tuple(msg.orig_address(), nb2hop_address);
								if (nb2hop_tuple == NULL) {
									nb2hop_tuple =
										new olsr_md_nb2hop_tuple;
									nb2hop_tuple->nb_main_address() =
										msg.orig_address();
									nb2hop_tuple->nb2hop_address() =
										nb2hop_address;
									add_nb2hop_tuple(nb2hop_tuple);
									nb2hop_tuple->time() =
										now + olsr_md::emf_to_seconds(msg.vtime());
									// Schedules nb2hop tuple
									// deletion
									olsr_md_Nb2hopTupleTimer* nb2hop_timer =
										new olsr_md_Nb2hopTupleTimer(this, nb2hop_tuple);
									nb2hop_timer->resched(DELAY(nb2hop_tuple->time()));
								}
								else {
									nb2hop_tuple->time() =
										now + olsr_md::emf_to_seconds(msg.vtime());
								}
								
							}
						}
						else if (nt == OLSR_MD_NOT_NEIGH) {
							// For each 2-hop node listed in the HELLO
							// message with Neighbor Type equal to
							// NOT_NEIGH all 2-hop tuples where:
							// N_neighbor_main_address == Originator
							// Address AND N_2hop_address  == main address
							// of the 2-hop neighbor are deleted.
							state_.erase_nb2hop_tuples(msg.orig_address(),
								nb2hop_address);
						}
					}
				}
			}
		}
	}
}

///
/// \brief	Updates the MPR Selector Set according to the information contained in a new
///		received HELLO message (following RFC 3626).
///
/// \param msg the %olsr_md message which contains the HELLO message.
///
void
olsr_md::populate_mprselset(olsr_md_msg& msg) {
	double now		= CURRENT_TIME;
	olsr_md_hello& hello	= msg.hello();
	
	assert(hello.count >= 0 && hello.count <= OLSR_MD_MAXI_HELLOS);
	for (int i = 0; i < hello.count; i++) {
		olsr_md_hello_msg& hello_msg = hello.hello_msg(i);
		int nt = hello_msg.lnk_code() >> 2;
		if (nt == OLSR_MD_MPR_NEIGH) {
			assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MD_MAXI_ADDRS);
			for (int j = 0; j < hello_msg.count; j++) {
				if (hello_msg.nb_intface_address(j) == ra_address()) {
					// We must create a new entry into the mpr selector set
					olsr_md_mprsel_tuple* mprsel_tuple =
						state_.find_mprsel_tuple(msg.orig_address());
					if (mprsel_tuple == NULL) {
						mprsel_tuple = new olsr_md_mprsel_tuple;
						mprsel_tuple->main_address() = msg.orig_address();
						mprsel_tuple->time() =
							now + olsr_md::emf_to_seconds(msg.vtime());
						add_mprsel_tuple(mprsel_tuple);
						// Schedules mpr selector tuple deletion
						olsr_md_MprSelTupleTimer* mprsel_timer =
							new olsr_md_MprSelTupleTimer(this, mprsel_tuple);
						mprsel_timer->resched(DELAY(mprsel_tuple->time()));
					}
					else
						mprsel_tuple->time() =
							now + olsr_md::emf_to_seconds(msg.vtime());
				}
			}
		}
	}
}

///
/// \brief	Drops a given packet because it couldn't be delivered to the corresponding
///		destination by the MAC layer. This may cause a neighbor loss, and appropiate
///		actions are then taken.
///
/// \param p the packet which couldn't be delivered by the MAC layer.
///
void
olsr_md::mac_failed(paquet* p) {
	double now		= CURRENT_TIME;
	struct hdm_ip* ih	= HDM_IP(p);
	struct hdm_cmn* ch	= HDM_CMN(p);
	
	printf("\n%f: Node %d MAC Layer detects a breakage on lnk to %d\n",
		now,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(ch->next_hop()));
	
	if ((u_int32_t)ih->daddress() == IP_BROADCAST) {
		drop(p, DROP_RTR_MAC_CALLBACK);
		return;
	}
	
	olsr_md_lnk_tuple* lnk_tuple = state_.find_lnk_tuple(ch->next_hop());
	if (lnk_tuple != NULL) {
		lnk_tuple->lost_time()	= now + OLSR_MD_NEIGHB_HOLD_TIME;
		lnk_tuple->time()	= now + OLSR_MD_NEIGHB_HOLD_TIME;
		nb_loss(lnk_tuple);
//		getchar();
	}

//	resend_data(p);
	drop(p, DROP_RTR_MAC_CALLBACK);
}

///
/// \brief Schedule the timer used for sending HELLO messages.
///
void
olsr_md::set_hello_timer() {
	hello_timer_.resched((double)(hello_ival() - JITTER));
}

///
/// \brief Schedule the timer used for sending TC messages.
///
void
olsr_md::set_tc_timer() {
	tc_timer_.resched((double)(tc_ival() - JITTER));
}

///
/// \brief Schedule the timer used for sending MID messages.
///
void
olsr_md::set_mid_timer() {
	mid_timer_.resched((double)(mid_ival() - JITTER));
}

///
/// \brief Performs all actions needed when a neighbor loss occurs.
///
/// Neighbor Set, 2-hop Neighbor Set, MPR Set and MPR Selector Set are updated.
///
/// \param tuple lnk tuple with the information of the lnk to the neighbor which has been lost.
///
void
olsr_md::nb_loss(olsr_md_lnk_tuple* tuple) {
	debug("%f: Node %d detects neighbor %d loss\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_intface_address()));
	
	//updated_lnk_tuple(tuple);
	//state_.erase_lnk_tuple(tuple);
	//state_.erase_nb_tuple(get_main_address(tuple->nb_intface_address()));
	rm_lnk_tuple(tuple);
	state_.erase_nb2hop_tuples(get_main_address(tuple->nb_intface_address()));
	state_.erase_mprsel_tuples(get_main_address(tuple->nb_intface_address()));
	
	mpr_computation();
	
 //	rtable_computation();//this is for unipath routing
 	
 	//for multipath routing, just set the out_of_date flag
 	m_rtbl_.set_flag(true);
}

///
/// \brief Adds a duplicate tuple to the Duplicate Set.
///
/// \param tuple the duplicate tuple to be added.
///
void
olsr_md::add_dup_tuple(olsr_md_dup_tuple* tuple) {
	/*debug("%f: Node %d adds dup tuple: address = %d seq_num = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->address()),
		tuple->seq_num());*/
	
	state_.insert_dup_tuple(tuple);
}

///
/// \brief Removes a duplicate tuple from the Duplicate Set.
///
/// \param tuple the duplicate tuple to be removed.
///
void
olsr_md::rm_dup_tuple(olsr_md_dup_tuple* tuple) {
	/*debug("%f: Node %d removes dup tuple: address = %d seq_num = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->address()),
		tuple->seq_num());*/
	
	state_.erase_dup_tuple(tuple);
}

///
/// \brief Adds a lnk tuple to the Link Set (and an associated neighbor tuple to the Neighbor Set).
///
/// \param tuple the lnk tuple to be added.
/// \param willingness willingness of the node which is going to be inserted in the Neighbor Set.
///
void
olsr_md::add_lnk_tuple(olsr_md_lnk_tuple* tuple, u_int8_t  willingness) {
	double now = CURRENT_TIME;

	debug("%f: Node %d adds lnk tuple: nb_address = %d\n",
		now,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_intface_address()));

	state_.insert_lnk_tuple(tuple);
	// Creates associated neighbor tuple
	olsr_md_nb_tuple* nb_tuple		= new olsr_md_nb_tuple;
	nb_tuple->nb_main_address()	= get_main_address(tuple->nb_intface_address());
	nb_tuple->willingness()		= willingness;
	if (tuple->sym_time() >= now)
		nb_tuple->status() = OLSR_MD_STATUS_SYM;
	else
		nb_tuple->status() = OLSR_MD_STATUS_NOT_SYM;
	add_nb_tuple(nb_tuple);
}

///
/// \brief Removes a lnk tuple from the Link Set.
///
/// \param tuple the lnk tuple to be removed.
///
void
olsr_md::rm_lnk_tuple(olsr_md_lnk_tuple* tuple) {
	nsaddress_t nb_address	= get_main_address(tuple->nb_intface_address());
	double now		= CURRENT_TIME;
	
	debug("%f: Node %d removes lnk tuple: nb_address = %d\n",
		now,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_intface_address()));
	// Prints this here cause we are not actually calling rm_nb_tuple() (efficiency stuff)
	debug("%f: Node %d removes neighbor tuple: nb_address = %d\n",
		now,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(nb_address));

	state_.erase_lnk_tuple(tuple);
	
	olsr_md_nb_tuple* nb_tuple = state_.find_nb_tuple(nb_address);
	state_.erase_nb_tuple(nb_tuple);
	delete nb_tuple;
//	delete tuple;
}

///
/// \brief	This function is invoked when a lnk tuple is updated. Its aim is to
///		also update the corresponding neighbor tuple if it is needed.
///
/// \param tuple the lnk tuple which has been updated.
///
void
olsr_md::updated_lnk_tuple(olsr_md_lnk_tuple* tuple) {
	double now = CURRENT_TIME;
	
	// Each time a lnk tuple changes, the associated neighbor tuple must be recomputed
	olsr_md_nb_tuple* nb_tuple =
		state_.find_nb_tuple(get_main_address(tuple->nb_intface_address()));
	if (nb_tuple != NULL) {
		if (USE_MAC&& tuple->lost_time() >= now)
			nb_tuple->status() = OLSR_MD_STATUS_NOT_SYM;
		else if (tuple->sym_time() >= now)
			nb_tuple->status() = OLSR_MD_STATUS_SYM;
		else
			nb_tuple->status() = OLSR_MD_STATUS_NOT_SYM;
	
	
	debug("%f: Node %d has updated lnk tuple: nb_address = %d status = %s\n",
		now,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_intface_address()),
		((nb_tuple->status() == OLSR_MD_STATUS_SYM) ? "sym" : "not_sym"));
		}
}

///
/// \brief Adds a neighbor tuple to the Neighbor Set.
///
/// \param tuple the neighbor tuple to be added.
///
void
olsr_md::add_nb_tuple(olsr_md_nb_tuple* tuple) {
	debug("%f: Node %d adds neighbor tuple: nb_address = %d status = %s\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_main_address()),
		((tuple->status() == OLSR_MD_STATUS_SYM) ? "sym" : "not_sym"));
	
	state_.insert_nb_tuple(tuple);
}

///
/// \brief Removes a neighbor tuple from the Neighbor Set.
///
/// \param tuple the neighbor tuple to be removed.
///
void
olsr_md::rm_nb_tuple(olsr_md_nb_tuple* tuple) {
	debug("%f: Node %d removes neighbor tuple: nb_address = %d status = %s\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_main_address()),
		((tuple->status() == OLSR_MD_STATUS_SYM) ? "sym" : "not_sym"));
	
	state_.erase_nb_tuple(tuple);
}

///
/// \brief Adds a 2-hop neighbor tuple to the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be added.
///
void
olsr_md::add_nb2hop_tuple(olsr_md_nb2hop_tuple* tuple) {
	debug("%f: Node %d adds 2-hop neighbor tuple: nb_address = %d nb2hop_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_main_address()),
		olsr_md::node_id(tuple->nb2hop_address()));

	state_.insert_nb2hop_tuple(tuple);
}

///
/// \brief Removes a 2-hop neighbor tuple from the 2-hop Neighbor Set.
///
/// \param tuple the 2-hop neighbor tuple to be removed.
///
void
olsr_md::rm_nb2hop_tuple(olsr_md_nb2hop_tuple* tuple) {
	debug("%f: Node %d removes 2-hop neighbor tuple: nb_address = %d nb2hop_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->nb_main_address()),
		olsr_md::node_id(tuple->nb2hop_address()));

	state_.erase_nb2hop_tuple(tuple);
}

///
/// \brief Adds an MPR selector tuple to the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be added.
///
void
olsr_md::add_mprsel_tuple(olsr_md_mprsel_tuple* tuple) {
	debug("%f: Node %d adds MPR selector tuple: nb_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->main_address()));

	state_.insert_mprsel_tuple(tuple);
	ansn_ = (ansn_ + 1)%(OLSR_MD_MAXI_SEQ_NUM + 1);
}

///
/// \brief Removes an MPR selector tuple from the MPR Selector Set.
///
/// Advertised Neighbor Sequence Number (ANSN) is also updated.
///
/// \param tuple the MPR selector tuple to be removed.
///
void
olsr_md::rm_mprsel_tuple(olsr_md_mprsel_tuple* tuple) {
	debug("%f: Node %d removes MPR selector tuple: nb_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->main_address()));

	state_.erase_mprsel_tuple(tuple);
	ansn_ = (ansn_ + 1)%(OLSR_MD_MAXI_SEQ_NUM + 1);
}

///
/// \brief Adds a topology tuple to the Topology Set.
///
/// \param tuple the topology tuple to be added.
///
void
olsr_md::add_topology_tuple(olsr_md_topology_tuple* tuple) {
	debug("%f: Node %d adds topology tuple: destination_address = %d last_address = %d seq = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->destination_address()),
		olsr_md::node_id(tuple->last_address()),
		tuple->seq());

	state_.insert_topology_tuple(tuple);
}

///
/// \brief Removes a topology tuple from the Topology Set.
///
/// \param tuple the topology tuple to be removed.
///
void
olsr_md::rm_topology_tuple(olsr_md_topology_tuple* tuple) {
	debug("%f: Node %d removes topology tuple: destination_address = %d last_address = %d seq = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->destination_address()),
		olsr_md::node_id(tuple->last_address()),
		tuple->seq());

	state_.erase_topology_tuple(tuple);
}

///
/// \brief Adds an interface association tuple to the Interface Association Set.
///
/// \param tuple the interface association tuple to be added.
///
void
olsr_md::add_intfaceassoc_tuple(olsr_md_intface_assoc_tuple* tuple) {
	debug("%f: Node %d adds intface association tuple: main_address = %d intface_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->main_address()),
		olsr_md::node_id(tuple->intface_address()));

	state_.insert_intfaceassoc_tuple(tuple);
}

///
/// \brief Removes an interface association tuple from the Interface Association Set.
///
/// \param tuple the interface association tuple to be removed.
///
void
olsr_md::rm_intfaceassoc_tuple(olsr_md_intface_assoc_tuple* tuple) {
	debug("%f: Node %d removes intface association tuple: main_address = %d intface_address = %d\n",
		CURRENT_TIME,
		olsr_md::node_id(ra_address()),
		olsr_md::node_id(tuple->main_address()),
		olsr_md::node_id(tuple->intface_address()));

	state_.erase_intfaceassoc_tuple(tuple);
}

///
/// \brief Gets the main address associated with a given interface address.
///
/// \param intface_address the interface address.
/// \return the corresponding main address.
///
nsaddress_t
olsr_md::get_main_address(nsaddress_t intface_address) {
	olsr_md_intface_assoc_tuple* tuple =
		state_.find_intfaceassoc_tuple(intface_address);
	
	if (tuple != NULL)
		return tuple->main_address();
	return intface_address;
}

///
/// \brief Determines which sequence number is bigger (as it is defined in RFC 3626).
///
/// \param s1 a sequence number.
/// \param s2 a sequence number.
/// \return true if s1 > s2, false in other case.
///
bool
olsr_md::seq_num_bigger_than(u_int16_t s1, u_int16_t s2) {
	return (s1 > s2 && s1-s2 <= OLSR_MD_MAXI_SEQ_NUM/2)
		|| (s2 > s1 && s2-s1 > OLSR_MD_MAXI_SEQ_NUM/2);
}

///
/// \brief This auxiliary function (defined in RFC 3626) is used for calculating the MPR Set.
///
/// \param tuple the neighbor tuple which has the main address of the node we are going to calculate its degree to.
/// \return the degree of the node.
///
int
olsr_md::degree(olsr_md_nb_tuple* tuple) {
	int degree = 0;
	for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
		olsr_md_nb2hop_tuple* nb2hop_tuple = *it;
		if (nb2hop_tuple->nb_main_address() == tuple->nb_main_address()) {
			olsr_md_nb_tuple* nb_tuple =
				state_.find_nb_tuple(nb2hop_tuple->nb_main_address());
			if (nb_tuple == NULL)
				degree++;
		}
	}
	return degree;
}

///
/// \brief Converts a decimal number of seconds to the mantissa/exponent format.
///
/// \param seconds decimal number of seconds we want to convert.
/// \return the number of seconds in mantissa/exponent format.
///
u_int8_t
olsr_md::seconds_to_emf(double seconds) {
	// This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
	// licensed under the GNU Public License (GPL)
	
	int a, b = 0;
 	while (seconds/OLSR_MD_C >= pow((double)2, (double)b))
		b++;
	b--;
	
	if (b < 0) {
		a = 1;
		b = 0;
	}
	else if (b > 15) {
		a = 15;
		b = 15;
	}
	else {
		a = (int)(16*((double)seconds/(OLSR_MD_C*(double)pow(2, b))-1));
		while (a >= 16) {
			a -= 16;
			b++;
		}
	}
	
	return (u_int8_t)(a*16+b);
}

///
/// \brief Converts a number of seconds in the mantissa/exponent format to a decimal number.
///
/// \param olsr_md_format number of seconds in mantissa/exponent format.
/// \return the decimal number of seconds.
///
double
olsr_md::emf_to_seconds(u_int8_t olsr_md_format) {
	// This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
	// licensed under the GNU Public License (GPL)
	int a = olsr_md_format >> 4;
	int b = olsr_md_format - a*16;
	return (double)(OLSR_MD_C*(1+(double)a/16)*(double)pow(2,b));
}

///
/// \brief Returns the identifier of a node given the address of the attached olsr_md agent.
///
/// \param address the address of the olsr_md routing agent.
/// \return the identifier of the node.
///
int
olsr_md::node_id(nsaddress_t address) {
	// Preventing a bad use for this function
        if ((u_int32_t)address == IP_BROADCAST)
		return address;
	// Getting node id
	Node* node = Node::get_node_by_addressess(address);
	assert(node != NULL);
	return node->nodeid();
}

///
///\brief Renew the weight of a topology tuple, for K-path Dijkstra
///
float
olsr_md::tuple_weight(float a){
	return 2*a+0.0;
}
