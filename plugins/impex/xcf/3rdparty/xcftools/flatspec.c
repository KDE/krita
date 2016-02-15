/* Flattening selections function for xcftools
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

#include "xcftools.h"
#include "flatten.h"
#include <string.h>
#include <stdlib.h>

void
init_flatspec(struct FlattenSpec *spec)
{
  spec->window_mode = USE_CANVAS ;
  spec->default_pixel = PERHAPS_ALPHA_CHANNEL ;
  spec->numLayers = 0 ;
  spec->layers = NULL ;
  spec->transmap_filename = NULL ;
  spec->output_filename = "-" ;
  spec->out_color_mode = COLOR_BY_CONTENTS ;
  spec->partial_transparency_mode = ALLOW_PARTIAL_TRANSPARENCY ;
  spec->process_in_memory = 0 ;
  spec->gimpish_indexed = 1 ;
}

void
add_layer_request(struct FlattenSpec *spec, const char *layer)
{
  spec->layers = realloc(spec->layers,
                         sizeof(struct xcfLayer) * (1+spec->numLayers));
  if( spec->layers == NULL )
    FatalUnexpected(_("Out of memory"));
  spec->layers[spec->numLayers].name = layer ;
  spec->layers[spec->numLayers].mode = (GimpLayerModeEffects)-1 ;
  spec->layers[spec->numLayers].opacity = 9999 ;
  spec->layers[spec->numLayers].hasMask = -1 ;
  spec->numLayers++ ;
}

struct xcfLayer *
lastlayerspec(struct FlattenSpec *spec,const char *option)
{
  if( spec->numLayers == 0 )
    FatalGeneric(20,_("The %s option must follow a layer name on the "
                      "command line"),option);
  return spec->layers + (spec->numLayers-1) ;
}

static int
typeHasTransparency(GimpImageType type)
{
  switch( type ) {
  case GIMP_RGB_IMAGE:
  case GIMP_GRAY_IMAGE:
  case GIMP_INDEXED_IMAGE:
    return 0 ;
  case GIMP_RGBA_IMAGE:
  case GIMP_GRAYA_IMAGE:
  case GIMP_INDEXEDA_IMAGE:
    return 1 ;
  }
  return 1 ;
}

static enum out_color_mode
color_by_layers(struct FlattenSpec *spec)
{
  int colormap_is_colored = 0 ;
  enum out_color_mode grayish ;
  int i ;

  if( spec->default_pixel == CHECKERED_BACKGROUND )
    grayish = COLOR_GRAY ;
  else {
    int degrayed = degrayPixel(spec->default_pixel);
    if( degrayed < 0 ) {
      return COLOR_RGB ;
    } else if( spec->gimpish_indexed &&
               (degrayed == 0 || degrayed == 255) ) {
      grayish = COLOR_MONO ;
    } else {
      grayish = COLOR_GRAY ;
    }
  }
  for( i=0; i<colormapLength; i++ ) {
    if( colormap[i] == NEWALPHA(0,0) || colormap[i] == NEWALPHA(-1,0) )
      continue ;
    if( degrayPixel(colormap[i]) == -1 ) {
      colormap_is_colored = 1 ;
      break ;
    } else {
      grayish = COLOR_GRAY ;
    }
  }
  for( i=0; i<spec->numLayers; i++ )
    switch( spec->layers[i].type ) {
    case GIMP_RGB_IMAGE:
    case GIMP_RGBA_IMAGE:
      return COLOR_RGB ;
    case GIMP_GRAY_IMAGE:
    case GIMP_GRAYA_IMAGE:
      grayish = COLOR_GRAY ;
      break ;
    case GIMP_INDEXED_IMAGE:
    case GIMP_INDEXEDA_IMAGE:
      if( colormap_is_colored ) return COLOR_RGB ;
      break ;
    }
  return grayish ;
}

void
complete_flatspec(struct FlattenSpec *spec, guesser guess_callback)
{
  unsigned i ;
  int anyPartial ;

  /* Find the layers to convert.
   */
  if( spec->numLayers == 0 ) {
    spec->layers = XCF.layers ;
    spec->numLayers = XCF.numLayers ;
  } else {
    for( i=0; i<spec->numLayers; i++ ) {
      GimpLayerModeEffects mode ;
      int opacity, hasMask ;
      unsigned j ;

      for( j=0; ; j++ ) {
        if( j == XCF.numLayers )
          FatalGeneric(22,_("The image has no layer called '%s'"),
                       spec->layers[i].name);
        if( strcmp(spec->layers[i].name,XCF.layers[j].name) == 0 )
          break ;
      }
      mode = spec->layers[i].mode == (GimpLayerModeEffects)-1 ?
        XCF.layers[j].mode : spec->layers[i].mode ;
      opacity = spec->layers[i].opacity == 9999 ?
        XCF.layers[j].opacity : spec->layers[i].opacity ;
      hasMask = spec->layers[i].hasMask == -1 ?
        XCF.layers[j].hasMask : spec->layers[i].hasMask ;
      if( hasMask && !XCF.layers[j].hasMask &&
          XCF.layers[j].mask.hierarchy == 0 )
        FatalGeneric(22,_("Layer '%s' has no layer mask to enable"),
                     spec->layers[i].name);
      spec->layers[i] = XCF.layers[j] ;
      spec->layers[i].mode = mode ;
      spec->layers[i].opacity = opacity ;
      spec->layers[i].hasMask = hasMask ;
      spec->layers[i].isVisible = 1 ;
    }
  }

  /* Force the mode of the lowest visible layer to be Normal or Dissolve.
   * That may not be logical, but the Gimp does it
   */
  for( i=0; i < spec->numLayers; i++ ) {
    if( spec->layers[i].isVisible ) {
      if( spec->layers[i].mode != GIMP_DISSOLVE_MODE )
        spec->layers[i].mode = GIMP_NORMAL_MODE ;
      break ;
    }
  }

  /* Mimic the Gimp's behavior on indexed layers */
  if( XCF.type == GIMP_INDEXED && spec->gimpish_indexed ) {
    for( i=0; i<spec->numLayers; i++ )
      if( spec->layers[i].mode != GIMP_DISSOLVE_MODE )
        spec->layers[i].mode = GIMP_NORMAL_NOPARTIAL_MODE ;
  } else
    spec->gimpish_indexed = 0 ;

  /* compute dimensions of the window */
  if( spec->window_mode == AUTOCROP ) {
    int first = 1 ;
    for( i=0; i<spec->numLayers; i++ )
      if( spec->layers[i].isVisible ) {
        computeDimensions(&spec->layers[i].dim) ;
        if( first ) {
          spec->dim = spec->layers[i].dim ;
          first = 0 ;
        } else {
          if( spec->dim.c.l > spec->layers[i].dim.c.l )
            spec->dim.c.l = spec->layers[i].dim.c.l ;
          if( spec->dim.c.r < spec->layers[i].dim.c.r )
            spec->dim.c.r = spec->layers[i].dim.c.r ;
          if( spec->dim.c.t > spec->layers[i].dim.c.t )
            spec->dim.c.t = spec->layers[i].dim.c.t ;
          if( spec->dim.c.b < spec->layers[i].dim.c.b )
            spec->dim.c.b = spec->layers[i].dim.c.b ;
        }
      }
    if( first ) {
      spec->window_mode = USE_CANVAS ;
    } else {
      spec->dim.width = spec->dim.c.r - spec->dim.c.l ;
      spec->dim.height = spec->dim.c.b - spec->dim.c.t ;
    }
  }
  if( spec->window_mode != AUTOCROP ) {
    if( (spec->window_mode & MANUAL_OFFSET) == 0 )
      spec->dim.c.t = spec->dim.c.l = 0 ;
    if( (spec->window_mode & MANUAL_CROP) == 0 ) {      
      spec->dim.height = XCF.height ;
      spec->dim.width = XCF.width ;
    }
  }
  computeDimensions(&spec->dim);
  
  /* Turn off layers that we don't hit at all */
  for( i=0; i<spec->numLayers; i++ )
    if( spec->layers[i].isVisible &&
        disjointRects(spec->dim.c,spec->layers[i].dim.c) )
      spec->layers[i].isVisible = 0 ;
  
  /* See if there is a completely covering layer somewhere in the stack */
  /* Also check if partial transparency is possible */
  anyPartial = 0 ;
  for( i=spec->numLayers; i-- ; ) {
    if( !spec->layers[i].isVisible )
      continue ;
    if( typeHasTransparency(spec->layers[i].type) ) {
      if( spec->layers[i].mode == GIMP_NORMAL_MODE )
        anyPartial = 1;
    } else if( isSubrect(spec->dim.c,spec->layers[i].dim.c) &&
               !spec->layers[i].hasMask &&
             (spec->layers[i].mode == GIMP_NORMAL_MODE ||
              spec->layers[i].mode == GIMP_NORMAL_NOPARTIAL_MODE ||
              spec->layers[i].mode == GIMP_DISSOLVE_MODE) ) {
      /* This layer fills out the entire image.
       * Turn off anly lower layers, and note that we cannot have
       * transparency at all.
       */
      while(i) spec->layers[--i].isVisible = 0 ;
      if( spec->default_pixel != FORCE_ALPHA_CHANNEL )
        spec->default_pixel = NEWALPHA(colormap[0],255);
      anyPartial = 0 ;
      break ;
    }
  }
  if( spec->partial_transparency_mode == ALLOW_PARTIAL_TRANSPARENCY &&
      (!anyPartial || ALPHA(spec->default_pixel) >= 128) )
    spec->partial_transparency_mode = PARTIAL_TRANSPARENCY_IMPOSSIBLE ;

  /* Initialize layers and print overview if we're verbose */
  for( i=spec->numLayers; i--; )
    if( spec->layers[i].isVisible ) {
      initLayer(&spec->layers[i]) ;
      if( verboseFlag ) {
        fprintf(stderr,"%dx%d%+d%+d %s %s",
                spec->layers[i].dim.width, spec->layers[i].dim.height,
                spec->layers[i].dim.c.l - spec->dim.c.l,
                spec->layers[i].dim.c.t - spec->dim.c.t,
                _(showGimpImageType(spec->layers[i].type)),
                _(showGimpLayerModeEffects(spec->layers[i].mode)));
        if( spec->layers[i].opacity < 255 )
          fprintf(stderr,"/%02d%%",spec->layers[i].opacity * 100 / 255);
        if( XCF.layers[i].hasMask )
          fprintf(stderr,_("/mask"));
        fprintf(stderr," %s\n",spec->layers[i].name);
      }
    }
  
  /* Resolve color mode unless we wait until we have the entire image */
  if( spec->out_color_mode == COLOR_BY_CONTENTS &&
      !spec->process_in_memory ) {
    if( guess_callback )
      spec->out_color_mode = guess_callback(spec,NULL);
    if( spec->out_color_mode == COLOR_BY_CONTENTS )
      spec->out_color_mode = color_by_layers(spec) ;
  }
}

void
analyse_colormode(struct FlattenSpec *spec,rgba **allPixels,
                  guesser guess_callback)
{
  unsigned x,y ;
  int status ;
  /* 8 - looking for any transparency
   * 4 - looking for partially transparent pixels
   * 2 - looking for pixels other than black and white
   * 1 - looking for colored pixels
   */
  int known_absent = 0 ;
  int assume_present = 0 ;

  if( spec->out_color_mode == COLOR_BY_CONTENTS && guess_callback )
    spec->out_color_mode = guess_callback(spec,allPixels) ;

  if( spec->out_color_mode == COLOR_RGB     ) assume_present |= 3 ;
  if( spec->out_color_mode == COLOR_INDEXED ) assume_present |= 3 ;
  if( spec->out_color_mode == COLOR_GRAY    ) assume_present |= 2 ;
  switch( color_by_layers(spec) ) {
  case COLOR_GRAY: known_absent |= 1 ; break ;
  case COLOR_MONO: known_absent |= 3 ; break ;
  default: break ;
  }
  if( spec->partial_transparency_mode == DISSOLVE_PARTIAL_TRANSPARENCY ||
      spec->partial_transparency_mode == PARTIAL_TRANSPARENCY_IMPOSSIBLE )
    known_absent |= 4 ;
  if( ALPHA(spec->default_pixel) >= 128 )               known_absent |= 12 ;
  else if( spec->default_pixel == FORCE_ALPHA_CHANNEL ) assume_present |= 8 ;

  status = 15 - (known_absent | assume_present) ;
  
  for( y=0; status && y<spec->dim.height; y++ ) {
    rgba *row = allPixels[y] ;
    if( (status & 3) != 0 ) {
      /* We're still interested in color */
      for( x=0; status && x<spec->dim.width; x++ ) {
        if( NULLALPHA(row[x]) )
          status &= ~8 ;
        else {
          rgba full = row[x] | (255 << ALPHA_SHIFT) ;
          if( !FULLALPHA(row[x]) ) status &= ~12 ;
          if( full == NEWALPHA(0,255) || full == NEWALPHA(-1,255) )
            /* Black or white */ ;
          else if( degrayPixel(row[x]) != -1 )
            status &= ~2 ; /* gray */
          else
            status &= ~3 ; /* color */
        }
      }
    } else {
      /* Not interested in color */
      for( x=0; status && x<spec->dim.width; x++ ) {
        if( NULLALPHA(row[x]) )
          status &= ~8 ;
        else if( !FULLALPHA(row[x]) )
          status &= ~12 ;
      }
    }
  }

  status |= known_absent ;
  
  switch( spec->out_color_mode ) {
  case COLOR_INDEXED: /* The caller takes responsibility */
  case COLOR_RGB: /* Everything is fine. */
    break ;
  case COLOR_GRAY:
    if( (status & 1) == 0 )
      FatalGeneric(103,
                   _("Grayscale output selected, but colored pixel(s) found"));
    break ;
  case COLOR_MONO:
    if( (status & 2) == 0 )
      FatalGeneric(103,_("Monochrome output selected, but not all pixels "
                         "are black or white"));
    break ;
  case COLOR_BY_FILENAME: /* Should not happen ... */
  case COLOR_BY_CONTENTS:
    if( (status & 1) == 0 )
      spec->out_color_mode = COLOR_RGB ;
    else if( (status & 2) == 0 )
      spec->out_color_mode = COLOR_GRAY ;
    else
      spec->out_color_mode = COLOR_MONO ;
    break ;
  }

  if( (status & 12) == 12 ) /* No transparency found */
    spec->default_pixel = NEWALPHA(colormap[0],255);
  else if( (status & 12) == 4 )
    spec->partial_transparency_mode = PARTIAL_TRANSPARENCY_IMPOSSIBLE ;
}
