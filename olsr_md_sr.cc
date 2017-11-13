/***************************************************************************           
*   Copyright (C) 2013													   *
*   TRSB MEDITERANNEE       							                   *
*   Programme de recherche           									   * 
***************************************************************************/
#include <stdio.h>
#include "olsr_md/olsr_md_sr.h"

int olsr_md_sr::offset_;

static class SRHeaderClass : public PaquetHeaderCls {
public:
	SRHeaderClass() : PaquetHeaderCls("PaquetHeader/SR",
					     sizeof(olsr_md_sr)) {
		offset(&olsr_md_sr::offset_);

#ifdef DSR_CONST_HDM_SZ
		fprintf(stderr,"WARNING: DSR treating all source route headers\n"
			"as having length %d. this should be used only to estimate effect\n"
			"of no longer needing a src rt in each paquet\n",SR_HDM_SZ);
#endif

	}
#if 0
	void export_offsets() {
		field_offset("available_", OFFSET(olsr_md_sr, available_));
		field_offset("num_address_", OFFSET(olsr_md_sr, num_address_));
		field_offset("cur_address_", OFFSET(olsr_md_sr, cur_address_));
	}
#endif
} class_SRhdr;

char *
olsr_md_sr::dump()
{
  static char buf[100];
  dump(buf);
  return (buf);
}

void
olsr_md_sr::dump(char *buf)
{
  char *ptr = buf;
  *ptr++ = '[';
  for (int i = 0; i < num_address_; i++)
    {
      ptr += sprintf(ptr, "%s%d ",
		     (i == cur_address_) ? "|" : "",
		     address()[i].address);
    }
  *ptr++ = ']';
  *ptr = '\0';
}
