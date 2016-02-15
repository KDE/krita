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

#ifndef PALETTE_H
#define PALETTE_H

#include "xcftools.h"
#include "pixels.h"

#define MAX_PALETTE 256
extern rgba palette[MAX_PALETTE] ;
extern unsigned paletteSize ;

typedef uint8_t index_t ;

void init_palette_hash(void);

/* lookup_or_intern() returns a negative number if there is no room
 * for the color in the palette.
 */
int lookup_or_intern(rgba color);

/* palettify_row will convert a row of 'rgba' values into a packed row
 * of 'uint8_t' indces. If it succeeds without running out of colormap
 * entries, it returns nonzero. On the other hand if it does run out
 * of colormap entries it returns zero _and_ undoes the conversions
 * already done, so that the row is still a full row of 'rgba' values
 * afterwards.
 */
int palettify_row(rgba *row,unsigned ncols);

/* palettify_rows is like palettify_rows, but works on several
 * rows at a time.
 */
int palettify_rows(rgba *rows[],unsigned ncols,unsigned nlines);

#endif /* PALETTE_H */
