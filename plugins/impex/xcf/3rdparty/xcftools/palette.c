/* Palette-manipulation functions functions for xcftools
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

#include "palette.h"
#include <assert.h>

#define HASH_SIZE 1711
/* If I remember correctly, this size hash will be able to handle
 * either of
 *  a) the Netscape cube with intensities 0x00, 0x33, 0x66, 0x99, 0xCC, xFF
 *  b) the EGA cube with intensities 0x00, 0x55, 0xAA, 0xFF
 *  c) the "CGA cube" with intensites 0x00, 0x80, 0xFF
 *  d) a full 256-step grayscale
 * without collisions. It will also have a minimal number of collisions
 * (4) on a full 16-to-a-side cube with intensities
 * 0x00, 0x11, 0x22, ..., 0xDD, 0xEE, 0xFF.
 */

unsigned paletteSize ;
rgba palette[MAX_PALETTE] ;
static int masterhash[HASH_SIZE];
static int bucketlinks[MAX_PALETTE];
    
void
init_palette_hash(void)
{
  unsigned i ;
  for( i=0; i<HASH_SIZE; i++ )
    masterhash[i] = -1 ;
  for( i=0; i<MAX_PALETTE; i++ )
    bucketlinks[i] = -1 ;
  paletteSize = 0 ;
}

inline int
lookup_or_intern(rgba color) {
  int *target = &masterhash[color % HASH_SIZE];
  while( *target >= 0 ) {
    if( palette[*target] == color )
      return *target ;
    target = &bucketlinks[*target] ;
  }
#if 0
  fprintf(stderr,"Palette[%u] = %08x (%u --> %d)\n",paletteSize,color,
          color % HASH_SIZE, *target);
#endif
  if( paletteSize >= MAX_PALETTE )
    return -1 ;
  *target = paletteSize ;
  palette[paletteSize] = color ;
  return paletteSize++ ;
}

static inline void
unpalettify_row(rgba *row,unsigned ncols)
{
  index_t *newrow = (index_t*) row ;
  unsigned i ;
  for( i=ncols; i--; ) {
    row[i] = palette[newrow[i]] ;
  }
}

int
palettify_row(rgba *row,unsigned ncols)
{
  index_t *newrow = (index_t*)row ;
  assert(sizeof(index_t) <= sizeof(rgba));
  unsigned i ;
  for( i=0; i<ncols; i++ ) {
    int j = lookup_or_intern(row[i]) ;
    if( j < 0 ) {
      unpalettify_row(row,i);
      return 0 ;
    }
    newrow[i] = j ;
  }
  return 1 ;
}
    
int
palettify_rows (rgba *rows[],unsigned ncols,unsigned nlines)
{
  unsigned i ;
  for( i=0; i<nlines; i++ ) {
    if( !palettify_row(rows[i],ncols) ) {
      while( i-- )
        unpalettify_row(rows[i],ncols);
      return 0 ;
    }
  }
  return 1 ;
}
        
  
