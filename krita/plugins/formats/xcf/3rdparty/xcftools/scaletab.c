/* Run-time scaletable computation for Xcftools
 *
 * This file was written by Henning Makholm <henning@makholm.net>
 * It is hereby in the public domain.
 * 
 * In jurisdictions that do not recognise grants of copyright to the
 * public domain: I, the author and (presumably, in those jurisdictions)
 * copyright holder, hereby permit anyone to distribute and use this code,
 * in source code or binary form, with or without modifications. This
 * permission is world-wide and irrevocable.
 *
 * Of course, I will not be liable for any errors or shortcomings in the
 * code, since I give it away without asking any compenstations.
 *
 * If you use or distribute this code, I would appreciate receiving
 * credit for writing it, in whichever way you find proper and customary.
 */

#include "pixels.h"
#ifndef PRECOMPUTED_SCALETABLE

uint8_t scaletable[256][256] ;
int ok_scaletable = 0 ;

void
mk_scaletable(void)
{
  unsigned p, q, r ;
  if( ok_scaletable ) return ;
  for( p = 0 ; p < 128 ; p++ )
    for( q = 0 ; q <= p ; q++  ) {
      r = (p*q+127)/255 ;
      scaletable[p][q] = scaletable[q][p] = r ;
      scaletable[255-p][q] = scaletable[q][255-p] = q-r ;
      scaletable[p][255-q] = scaletable[255-q][p] = p-r ;
      scaletable[255-p][255-q] = scaletable[255-q][255-p] = (255-q)-(p-r) ;
    }
  ok_scaletable = 1 ;
}

#endif
    
