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
/// \file	olsr_md_repositories.h
/// \Ici sont définies toutes les structures de données nécessaires à un noeud olsr_md.
///

#ifndef __olsr_md_repositories_h__
#define __olsr_md_repositories_h__

#include "set"
#include "vector"
#include "config.h"
#include "olsr_md_sr.h"

#define MAXI_WEIGHT	65535	//poids maximum pour le noeud

/// Une entrée de table de routage pour le routage par trajets multiples
//typedef std::vector<nsaddress_t>	olsr_md_m_rt_entry;
typedef struct olsr_md_m_rt_entry{
	sr_address	address_[MAXI_SR_LEN];
}olsr_md_m_rt_entry;

/// Une entrée de table de routage de olsr_md.
typedef struct olsr_md_rt_entry {
	nsaddress_t	destination_address_;	///< Adresse du noeud de destination.
	nsaddress_t	next_address_;	///< Adresse du prochain saut
	nsaddress_t	intface_address_;	///< Adresse de l'interface locale.
	u_int32_t	dist_;		///< Distance en sauts à la destination.
	
	inline nsaddress_t&	destination_address()	{ return destination_address_; }
	inline nsaddress_t&	next_address()	{ return next_address_; }
	inline nsaddress_t&	intface_address()	{ return intface_address_; }
	inline u_int32_t&	dist()		{ return dist_; }
} olsr_md_rt_entry;

/// Un Tuple d'association d'interface.
typedef struct olsr_md_intface_assoc_tuple {
	/// Adresse de l'interface d'un noeud.
	nsaddress_t	intface_address_;
	/// Adresse principale du noeud.
	nsaddress_t	main_address_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	
	inline nsaddress_t&	intface_address()	{ return intface_address_; }
	inline nsaddress_t&	main_address()	{ return main_address_; }
	inline double&		time()		{ return time_; }
} olsr_md_intface_assoc_tuple;


/// une structure de noeud pour Dijkstra
typedef struct Dijkstra_node{
	nsaddress_t	address_;
	nsaddress_t	pre_address_;
	float		weight_;
	int 		node_type;// 0 pour T type, 1 pour P type
} Dijkstra_node;


/// Un lien Tuple.
typedef struct olsr_md_lnk_tuple {
	/// Les adresses d'interface du noeud local.
	nsaddress_t	local_intface_address_;
	/// Les adresses d'interface du noeud voisin.
	nsaddress_t	nb_intface_address_;
	/// Le lien est considéré bidirectionnel jusqu'à ce moment.
	double		sym_time_;
	/// Le lien est considéré comme unidirectionnel jusqu'à ce moment.
	double		asym_time_;
	/// Le lien est considéré perdu jusqu'à ce moment (utilisé pour la notification de la couche liaison).
	double		lost_time_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	
	inline nsaddress_t&	local_intface_address()	{ return local_intface_address_; }
	inline nsaddress_t&	nb_intface_address()		{ return nb_intface_address_; }
	inline double&		sym_time()		{ return sym_time_; }
	inline double&		asym_time()		{ return asym_time_; }
	inline double&		lost_time()		{ return lost_time_; }
	inline double&		time()			{ return time_; }
} olsr_md_lnk_tuple;

///Un Tuple voisin.
typedef struct olsr_md_nb_tuple {
	/// Adresse principale d'un noeud voisin.
	nsaddress_t nb_main_address_;
	/// Type de voisin et type de lien aux quatre digits moins significatifs.
	u_int8_t status_;
	/// Une valeur comprise entre 0 et 7 spécifiant la volonté du nœud de transporter du trafic pour le compte d'autres nœuds.
	u_int8_t willingness_;
	
	inline nsaddress_t&	nb_main_address()	{ return nb_main_address_; }
	inline u_int8_t&	status()	{ return status_; }
	inline u_int8_t&	willingness()	{ return willingness_; }
} olsr_md_nb_tuple;

/// Un Tuple à 2 sauts.
typedef struct olsr_md_nb2hop_tuple {
	/// Adresse principale d'un voisin.
	nsaddress_t	nb_main_address_;
	/// Adresse principale d'un voisin à deux sauts avec un lien symétrique vers nb_main_address.
	nsaddress_t	nb2hop_address_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	
	inline nsaddress_t&	nb_main_address()	{ return nb_main_address_; }
	inline nsaddress_t&	nb2hop_address()	{ return nb2hop_address_; }
	inline double&		time()		{ return time_; }
} olsr_md_nb2hop_tuple;

/// Un tuple selecteur
typedef struct olsr_md_mprsel_tuple {
	/// Adresse principale d'un noeud ayant sélectionné ce noeud en tant que MPR.
	nsaddress_t	main_address_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	
	inline nsaddress_t&	main_address()	{ return main_address_; }
	inline double&		time()		{ return time_; }
} olsr_md_mprsel_tuple;

/// Le type "liste des adresses d'interface"
typedef std::vector<nsaddress_t> address_list_t;

/// Un tuple en double
typedef struct olsr_md_dup_tuple {
	/// Adresse de l'expéditeur du message
	nsaddress_t	address_;
	/// Numéro de séquence du message.
	u_int16_t	seq_num_;
	/// Indique si le message a été retransmis ou non.
	bool		retransmitted_;
	/// Liste des interfaces sur lesquelles le message a été reçu.
	address_list_t	intface_list_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	
	inline nsaddress_t&	address()		{ return address_; }
	inline u_int16_t&	seq_num()	{ return seq_num_; }
	inline bool&		retransmitted()	{ return retransmitted_; }
	inline address_list_t&	intface_list()	{ return intface_list_; }
	inline double&		time()		{ return time_; }
} olsr_md_dup_tuple;

/// Un tuple de topologie
typedef struct olsr_md_topology_tuple {
	/// Adresse principale de la destination.
	nsaddress_t	destination_address_;
	/// Adresse principale d'un noeud qui est voisin de la destination.
	nsaddress_t	last_address_;
	/// Numéro de séquence.
	u_int16_t	seq_;
	/// Moment auquel ce tuple expire et doit être supprimé.
	double		time_;
	/// Poids entre le noeud de destination et le dernier noeud
	float		weight_;
	/// poids de l'origine
	float		original_weight_;
	
	inline nsaddress_t&	destination_address()	{ return destination_address_; }
	inline nsaddress_t&	last_address()	{ return last_address_; }
	inline u_int16_t&	seq()		{ return seq_; }
	inline double&		time()		{ return time_; }
	inline float&		weight()	{ return weight_;}
	inline float&		original_weight()	{return original_weight_;}
} olsr_md_topology_tuple;



typedef std::set<nsaddress_t>					mpr_set_t;		
typedef std::vector<olsr_md_mprsel_tuple*>		mpr_selset_t;	
typedef std::vector<olsr_md_lnk_tuple*>			lnkset_t;		
typedef std::vector<olsr_md_nb_tuple*>			nbset_t;	
typedef std::vector<olsr_md_nb2hop_tuple*>		nb2hopset_t;	
typedef std::vector<olsr_md_topology_tuple*>	topologyset_t;	
typedef std::vector<olsr_md_dup_tuple*>			dupset_t;		
typedef std::vector<olsr_md_intface_assoc_tuple*>	intfaceassocset_t;

#endif
