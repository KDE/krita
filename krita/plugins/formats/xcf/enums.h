/* Extracted from
 *   gimp/base-enums.h
 *   gimp/gimpbaseenums.h
 *   gimp/xcf-private.h
 * by mkenumsh.pl
 */
typedef enum
{
  GIMP_NORMAL_MODE,
  GIMP_DISSOLVE_MODE,
  GIMP_BEHIND_MODE,
  GIMP_MULTIPLY_MODE,
  GIMP_SCREEN_MODE,
  GIMP_OVERLAY_MODE,
  GIMP_DIFFERENCE_MODE,
  GIMP_ADDITION_MODE,
  GIMP_SUBTRACT_MODE,
  GIMP_DARKEN_ONLY_MODE,
  GIMP_LIGHTEN_ONLY_MODE,
  GIMP_HUE_MODE,
  GIMP_SATURATION_MODE,
  GIMP_COLOR_MODE,
  GIMP_VALUE_MODE,
  GIMP_DIVIDE_MODE,
  GIMP_DODGE_MODE,
  GIMP_BURN_MODE,
  GIMP_HARDLIGHT_MODE,
  GIMP_SOFTLIGHT_MODE,
  GIMP_GRAIN_EXTRACT_MODE,
  GIMP_GRAIN_MERGE_MODE,
  GIMP_COLOR_ERASE_MODE,
  GIMP_ERASE_MODE,           /*< pdb-skip, skip >*/
  GIMP_REPLACE_MODE,         /*< pdb-skip, skip >*/
  GIMP_ANTI_ERASE_MODE       /*< pdb-skip, skip >*/
  ,GIMP_NORMAL_NOPARTIAL_MODE=-1
} GimpLayerModeEffects;
const char *showGimpLayerModeEffects(GimpLayerModeEffects);
#define GimpLayerModeEffects_LAST GIMP_ANTI_ERASE_MODE
typedef enum
{
  GIMP_RGB,     /*< desc="RGB color"     >*/
  GIMP_GRAY,    /*< desc="Grayscale"     >*/
  GIMP_INDEXED  /*< desc="Indexed color" >*/
} GimpImageBaseType;
const char *showGimpImageBaseType(GimpImageBaseType);
#define GimpImageBaseType_LAST GIMP_INDEXED
typedef enum
{
  GIMP_RGB_IMAGE,      /*< desc="RGB"             >*/
  GIMP_RGBA_IMAGE,     /*< desc="RGB-alpha"       >*/
  GIMP_GRAY_IMAGE,     /*< desc="Grayscale"       >*/
  GIMP_GRAYA_IMAGE,    /*< desc="Grayscale-alpha" >*/
  GIMP_INDEXED_IMAGE,  /*< desc="Indexed"         >*/
  GIMP_INDEXEDA_IMAGE  /*< desc="Indexed-alpha"   >*/
} GimpImageType;
const char *showGimpImageType(GimpImageType);
#define GimpImageType_LAST GIMP_INDEXEDA_IMAGE
typedef enum
{
  PROP_END                   =  0,
  PROP_COLORMAP              =  1,
  PROP_ACTIVE_LAYER          =  2,
  PROP_ACTIVE_CHANNEL        =  3,
  PROP_SELECTION             =  4,
  PROP_FLOATING_SELECTION    =  5,
  PROP_OPACITY               =  6,
  PROP_MODE                  =  7,
  PROP_VISIBLE               =  8,
  PROP_LINKED                =  9,
  PROP_PRESERVE_TRANSPARENCY = 10,
  PROP_APPLY_MASK            = 11,
  PROP_EDIT_MASK             = 12,
  PROP_SHOW_MASK             = 13,
  PROP_SHOW_MASKED           = 14,
  PROP_OFFSETS               = 15,
  PROP_COLOR                 = 16,
  PROP_COMPRESSION           = 17,
  PROP_GUIDES                = 18,
  PROP_RESOLUTION            = 19,
  PROP_TATTOO                = 20,
  PROP_PARASITES             = 21,
  PROP_UNIT                  = 22,
  PROP_PATHS                 = 23,
  PROP_USER_UNIT             = 24,
  PROP_VECTORS               = 25,
  PROP_TEXT_LAYER_FLAGS      = 26
} PropType;
const char *showPropType(PropType);
#define PropType_LAST PROP_TEXT_LAYER_FLAGS
typedef enum
{
  COMPRESS_NONE              =  0,
  COMPRESS_RLE               =  1,
  COMPRESS_ZLIB              =  2,  /* unused */
  COMPRESS_FRACTAL           =  3   /* unused */
} XcfCompressionType;
const char *showXcfCompressionType(XcfCompressionType);
#define XcfCompressionType_LAST COMPRESS_FRACTAL
