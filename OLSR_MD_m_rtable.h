/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

///
///Fichier d'en-tête pour la table de routage 
///
#ifndef __olsr_md_m_rtbl_h__
#define __olsr_md_m_rtbl_h__
//#include <olsr_md/olsr_md.h>
#include "olsr_md_repositories.h"
#include <trace.h>
#include <map>
#include "cmu-trace.h"

///
///définit molsr_tbl_t comme une carte de olsr_md_m_rt_entry, dont la clé est la destination.
///La table de routage est donc définie en tant que paires: [dest adsressess, entry].
//typedef std::map<nsaddress_t, olsr_md_m_rt_entry*> molsr_tbl_t;


//typedef std::vector<olsr_md_m_rt_entry*> 	molsr_tbl_t;
typedef std::multimap<nsaddress_t,olsr_md_m_rt_entry*> molsr_tbl_t;


///
///Cette classe est une représentation de la table de routage multipath de olsr_md
///
class olsr_md_m_rtbl{
	molsr_tbl_t molsr_rt_;
	
	//le flag, pour voir si la table doit être recalculée.
	bool out_of_date[MAXI_NODE];

public:
	olsr_md_m_rtbl();
	~olsr_md_m_rtbl();

	molsr_tbl_t* molsr_rt();
	void 	set_flag(int id,bool flag);
	void 	set_flag(bool flag);
	bool	get_flag(int id);
	void 	clear();
	void 	rm_entry(nsaddress_t des);
	void	add_entry(olsr_md_m_rt_entry* entry,nsaddress_t address);
	molsr_tbl_t::iterator 	lookup(nsaddress_t dest);
	
	u_int32_t	size();
	void 	print(Trace*);

};

#endif
