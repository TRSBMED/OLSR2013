/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/

#ifndef _MDPAQUET_H_
#define _MDPAQUET_H_

#include <paquet.h>
#include "olsr_md_sr.h"

#include "path.h"

struct MDpaquet {

  ID dest;
  ID src;
  paquet *pqt;
  Path route;
  MDpaquet(paquet *p, struct olsr_md_sr *srh) : pqt(p), route(srh) {}
  MDpaquet() : pqt(NULL) {}
};

#endif  //_MDPAQUET_H_
