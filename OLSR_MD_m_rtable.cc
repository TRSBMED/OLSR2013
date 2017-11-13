/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#include "olsr_md/olsr_md_m_rtbl.h"
#include "olsr_md/olsr_md_repositories.h"
using namespace std;
///
///Crée une nouvelle table de routage vide
///
olsr_md_m_rtbl::olsr_md_m_rtbl(){
	for (int i = 0;i<MAXI_NODE; i ++)
	{
		out_of_date[i] = true;
	}
}

///
///Détruit la table de routage et toutes ses entrées
///
olsr_md_m_rtbl::~olsr_md_m_rtbl(){
	for(molsr_tbl_t::iterator it = molsr_rt_.begin();it!=molsr_rt_.end();it++)
		delete (*it).second;
		
}

molsr_tbl_t* olsr_md_m_rtbl::molsr_rt(){
	return &molsr_rt_;
}
void olsr_md_m_rtbl::set_flag(int id, bool flag){
	out_of_date[id] = flag;
}

void olsr_md_m_rtbl::set_flag(bool flag){
	for (int i = 0; i < MAXI_NODE; i ++){
		out_of_date[i] = flag;
	}
}

bool olsr_md_m_rtbl::get_flag(int id){
	return out_of_date[id];
}
///
///Détruit la table de routage et toutes ses entrées
///
void olsr_md_m_rtbl::clear(){
	for (molsr_tbl_t::iterator it = molsr_rt_.begin();it != molsr_rt_.end(); it++)
		delete (*it).second;

	molsr_rt_.clear();
}


///
///Supprime l'entrée dont la destination est donnée.
///les adresses des paramètres des noeuds de destination
///
void olsr_md_m_rtbl::rm_entry(nsaddress_t dest){
/*	for (molsr_tbl_t::iterator it = molsr_rt_.begin(); it != molsr_rt_.end(); it++){
		if ( (olsr_md_m_rt_entry*)((*it).second).end() == dest)
			delete (*it).second;
	}
*/
	molsr_rt_.erase(dest);
	
}

///
///trouve les entrées pour les adresses de destination spécifiées
///
molsr_tbl_t::iterator olsr_md_m_rtbl::lookup(nsaddress_t dest){
	return molsr_rt_.find(dest);
}

///
///ajouter une nouvelle entrée à la table de routage
///
void olsr_md_m_rtbl::add_entry(olsr_md_m_rt_entry* entry,nsaddress_t address){
/*
	olsr_md_m_rt_entry::iterator it = (*entry).end();
	nsaddress_t address = (*it);
	molsr_rt_.insert(pair<nsaddress_t,olsr_md_m_rt_entry*>(address,entry));*/
//	nsaddress_t address = (*entry).address
	molsr_rt_.insert(pair<nsaddress_t,olsr_md_m_rt_entry*>(address,entry));
}
