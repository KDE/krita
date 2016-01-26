/* Flattning functions for xcftools
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

#ifndef FLATTEN_H
#define FLATTEN_H

#include "xcftools.h"
#include "pixels.h"

#define PERHAPS_ALPHA_CHANNEL (NEWALPHA(0,1))
#define FORCE_ALPHA_CHANNEL (NEWALPHA(0,2))
#define CHECKERED_BACKGROUND (NEWALPHA(0,200))
struct FlattenSpec {
  struct tileDimensions dim ;
  rgba default_pixel ;
  int numLayers ;
  struct xcfLayer *layers ;

  const char * transmap_filename ;
  const char * output_filename ;
  enum out_color_mode {
    COLOR_BY_FILENAME,
    COLOR_BY_CONTENTS,
    COLOR_INDEXED,
    COLOR_RGB,
    COLOR_GRAY,
    COLOR_MONO
  } out_color_mode ;
  enum { ALLOW_PARTIAL_TRANSPARENCY,
         DISSOLVE_PARTIAL_TRANSPARENCY,
         FORBID_PARTIAL_TRANSPARENCY,
         PARTIAL_TRANSPARENCY_IMPOSSIBLE
  } partial_transparency_mode ;
  enum { USE_CANVAS = 0,
         MANUAL_OFFSET = 1,
         MANUAL_CROP = 2,
         AUTOCROP = 4 } window_mode ;
  int process_in_memory ;
  int gimpish_indexed ;
};

/* From flatspec.c */

void init_flatspec(struct FlattenSpec *);

void add_layer_request(struct FlattenSpec *,const char *name);
struct xcfLayer *lastlayerspec(struct FlattenSpec *,const char *option);

typedef enum out_color_mode (*guesser) (struct FlattenSpec *,rgba **);

/* Call this after processing options, and after opening the XCF file */
void complete_flatspec(struct FlattenSpec *,guesser);
void analyse_colormode(struct FlattenSpec *,rgba **allPixels,guesser);

/* From flatten.c */

typedef void (*lineCallback)(unsigned num,rgba *pixels);
void flattenIncrementally(struct FlattenSpec *,lineCallback);
rgba **flattenAll(struct FlattenSpec*);
void shipoutWithCallback(struct FlattenSpec *,rgba **pixels,lineCallback);

#endif /* FLATTEN_H */
