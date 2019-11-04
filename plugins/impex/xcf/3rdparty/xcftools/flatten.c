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

#include "xcftools.h"
#include "flatten.h"
#include "pixels.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static rgba __ATTRIBUTE__((noinline,const))
composite_one(rgba bot,rgba top)
{
    unsigned tfrac, alpha ;

    tfrac = ALPHA(top) ;
    alpha = 255 ;
    if( !FULLALPHA(bot) ) {
        alpha = 255 ^ scaletable[255-ALPHA(bot)][255-ALPHA(top)] ;
        /* This peculiar combination of ^ and - makes the GCC code
     * generator for i386 particularly happy.
     */
        tfrac = (256*ALPHA(top) - 1) / alpha ;
        /* Tfrac is the fraction of the coposited pixel's covered area
     * that comes from the top pixel.
     * For mathematical accuracy we ought to scale by 255 and
     * subtract alpha/2, but this is faster, and never misses the
     * true value by more than one 1/255. This effect is completely
     * overshadowed by the linear interpolation in the first place.
     * (I.e. gamma is ignored when combining intensities).
     *   [In any case, complete fairness is not possible: if the
     *    bottom pixel had alpha=170 and the top has alpha=102,
     *    each should contribute equally to the color of the
     *    resulting alpha=204 pixel, which is not possible in general]
     * Subtracting one helps the topfrac never be 256, which would
     * be bad.
     * On the other hand it means that we would get tfrac=-1 if the
     * top pixel is completely transparent, and we get a division
     * by zero if _both_ pixels are fully transparent. These cases
     * must be handled by all callers.
     *    More snooping in the Gimp sources reveal that it uses
     *    floating-point for its equivalent of tfrac when the
     *    bottom layer has an alpha channel. (alphify() macro
     *    in paint-funcs.c). What gives?
     */
    }
    return (alpha << ALPHA_SHIFT)
            + ((uint32_t)scaletable[  tfrac  ][255&(top>>RED_SHIFT  )] << RED_SHIFT   )
            + ((uint32_t)scaletable[  tfrac  ][255&(top>>GREEN_SHIFT)] << GREEN_SHIFT )
            + ((uint32_t)scaletable[  tfrac  ][255&(top>>BLUE_SHIFT )] << BLUE_SHIFT  )
            + ((uint32_t)scaletable[255^tfrac][255&(bot>>RED_SHIFT  )] << RED_SHIFT   )
            + ((uint32_t)scaletable[255^tfrac][255&(bot>>GREEN_SHIFT)] << GREEN_SHIFT )
            + ((uint32_t)scaletable[255^tfrac][255&(bot>>BLUE_SHIFT )] << BLUE_SHIFT  )
            ;
}

/* merge_normal() takes ownership of bot.
 * merge_normal() will share ownership of top.
 * Return: may be shared.
 */
static struct Tile * __ATTRIBUTE__((noinline))
merge_normal(struct Tile *bot, struct Tile *top)
{
    unsigned i ;
    assertTileCompatibility(bot,top);

    /* See if there is an easy winner */
    if( (bot->summary & TILESUMMARY_ALLNULL) ||
            (top->summary & TILESUMMARY_ALLFULL) ) {
        freeTile(bot);
        return top ;
    }
    if( top->summary & TILESUMMARY_ALLNULL ) {
        freeTile(top);
        return bot ;
    }

    /* Try hard to make top win */
    for( i=0; ; i++ ) {
        if( i == top->count ) {
            freeTile(bot);
            return top ;
        }
        if( !(NULLALPHA(bot->pixels[i]) || FULLALPHA(top->pixels[i])) )
            break ;
    }

    INIT_SCALETABLE_IF( !(top->summary & TILESUMMARY_CRISP) );

    /* Otherwise bot wins, but is forever changed ... */
    if( (top->summary & TILESUMMARY_ALLNULL) == 0 ) {
        unsigned i ;
        invalidateSummary(bot,0);
        for( i=0 ; i < top->count ; i++ ) {
            if( !NULLALPHA(top->pixels[i]) ) {
                if( FULLALPHA(top->pixels[i]) || NULLALPHA(bot->pixels[i]) )
                    bot->pixels[i] = top->pixels[i] ;
                else
                    bot->pixels[i] = composite_one(bot->pixels[i],top->pixels[i]);
            }
        }
    }
    freeTile(top);
    return bot ;
}

#define exotic_combinator static unsigned __ATTRIBUTE__((const))



exotic_combinator
ucombine_ADDITION(uint8_t bot,uint8_t top)
{
    return bot+top > 255 ? 255 : bot+top ;
}

exotic_combinator
ucombine_SUBTRACT(uint8_t bot,uint8_t top)
{
    return top>bot ? 0 : bot-top ;
}

exotic_combinator
ucombine_LIGHTEN_ONLY(uint8_t bot,uint8_t top)
{
    return top > bot ? top : bot ;
}

exotic_combinator
ucombine_DARKEN_ONLY(uint8_t bot,uint8_t top)
{
    return top < bot ? top : bot ;
}

exotic_combinator
ucombine_DIFFERENCE(uint8_t bot,uint8_t top)
{
    return top > bot ? top-bot : bot-top ;
}

exotic_combinator
ucombine_MULTIPLY(uint8_t bot,uint8_t top)
{
    return scaletable[bot][top] ;
}

exotic_combinator
ucombine_DIVIDE(uint8_t bot,uint8_t top)
{
    int result = (int)bot*256 / (1+top) ;
    return result >= 256 ? 255 : result ;
}

exotic_combinator
ucombine_SCREEN(uint8_t bot,uint8_t top)
{
    /* An inverted version of "multiply" */
    return 255 ^ scaletable[255-bot][255-top] ;
}

exotic_combinator
ucombine_OVERLAY(uint8_t bot,uint8_t top)
{
    return scaletable[bot][bot] +
            2*scaletable[top][scaletable[bot][255-bot]] ;
    /* This strange formula is equivalent to
   *   (1-top)*(bot^2) + top*(1-(1-top)^2)
   * that is, the top value is used to interpolate between
   * the self-multiply and the self-screen of the bottom.
   */
    /* Note: This is exactly what the "Soft light" effect also
   * does, though with different code in the Gimp.
   */
}

exotic_combinator
ucombine_DODGE(uint8_t bot,uint8_t top)
{
    return ucombine_DIVIDE(bot,255-top);
}

exotic_combinator
ucombine_BURN(uint8_t bot,uint8_t top)
{
    return 255 - ucombine_DIVIDE(255-bot,top);
}

exotic_combinator
ucombine_HARDLIGHT(uint8_t bot,uint8_t top)
{
    if( top >= 128 )
        return 255 ^ scaletable[255-bot][2*(255-top)] ;
    else
        return scaletable[bot][2*top];
    /* The code that implements "hardlight" in Gimp 2.2.10 has some
   * rounding errors, but this is undoubtedly what is meant.
   */
}

exotic_combinator
ucombine_GRAIN_EXTRACT(uint8_t bot,uint8_t top)
{
    int temp = (int)bot - (int)top + 128 ;
    return temp < 0 ? 0 : temp >= 256 ? 255 : temp ;
}

exotic_combinator
ucombine_GRAIN_MERGE(uint8_t bot,uint8_t top)
{
    int temp = (int)bot + (int)top - 128 ;
    return temp < 0 ? 0 : temp >= 256 ? 255 : temp ;
}

struct HSV {
    enum { HUE_RED_GREEN_BLUE,HUE_RED_BLUE_GREEN,HUE_BLUE_RED_GREEN,
           HUE_BLUE_GREEN_RED,HUE_GREEN_BLUE_RED,HUE_GREEN_RED_BLUE } hue;
    unsigned ch1, ch2, ch3 ;
};

static void
RGBtoHSV(rgba rgb,struct HSV *hsv)
{
    unsigned RED = (uint8_t)(rgb >> RED_SHIFT);
    unsigned GREEN = (uint8_t)(rgb >> GREEN_SHIFT);
    unsigned BLUE = (uint8_t)(rgb >> BLUE_SHIFT) ;
#define HEXTANT(b,m,t) hsv->ch1 = b, hsv->ch2 = m, hsv->ch3 = t, \
    hsv->hue = HUE_ ## b ## _ ## m ## _ ## t
    if( GREEN <= RED )
        if( BLUE <= RED )
            if( GREEN <= BLUE )
                HEXTANT(GREEN,BLUE,RED);
            else
                HEXTANT(BLUE,GREEN,RED);
        else
            HEXTANT(GREEN,RED,BLUE);
    else if( BLUE <= RED )
        HEXTANT(BLUE,RED,GREEN);
    else if( BLUE <= GREEN )
        HEXTANT(RED,BLUE,GREEN);
    else
        HEXTANT(RED,GREEN,BLUE);
#undef HEXTANT
}

/* merge_exotic() destructively updates bot.
 * merge_exotic() reads but does not free top.
 */
static int __ATTRIBUTE__((noinline))
merge_exotic(struct Tile *bot, const struct Tile *top,
             GimpLayerModeEffects mode)
{
    unsigned i ;
    assertTileCompatibility(bot,top);
    if( (bot->summary & TILESUMMARY_ALLNULL) != 0 ) return XCF_OK;
    if( (top->summary & TILESUMMARY_ALLNULL) != 0 ) return XCF_OK;
    assert( bot->refcount == 1 );
    /* The transparency status of bot never changes */

    INIT_SCALETABLE_IF(1);

    for( i=0; i < top->count ; i++ ) {
        uint32_t RED, GREEN, BLUE ;
        if( NULLALPHA(bot->pixels[i]) || NULLALPHA(top->pixels[i]) )
            continue ;
#define UNIFORM(mode) case GIMP_ ## mode ## _MODE: \
    RED   = ucombine_ ## mode (bot->pixels[i]>>RED_SHIFT  ,  \
    top->pixels[i]>>RED_SHIFT  ); \
    GREEN = ucombine_ ## mode (bot->pixels[i]>>GREEN_SHIFT,  \
    top->pixels[i]>>GREEN_SHIFT); \
    BLUE  = ucombine_ ## mode (bot->pixels[i]>>BLUE_SHIFT ,  \
    top->pixels[i]>>BLUE_SHIFT ); \
    break ;
        switch( mode ) {
        case GIMP_NORMAL_MODE:
        case GIMP_DISSOLVE_MODE:
            {
                FatalUnexpected("Normal and Dissolve mode can't happen here!");
                return XCF_ERROR;
            }
            UNIFORM(ADDITION);
            UNIFORM(SUBTRACT);
            UNIFORM(LIGHTEN_ONLY);
            UNIFORM(DARKEN_ONLY);
            UNIFORM(DIFFERENCE);
            UNIFORM(MULTIPLY);
            UNIFORM(DIVIDE);
            UNIFORM(SCREEN);
        case GIMP_SOFTLIGHT_MODE: /* A synonym for "overlay"! */
            UNIFORM(OVERLAY);
            UNIFORM(DODGE);
            UNIFORM(BURN);
            UNIFORM(HARDLIGHT);
            UNIFORM(GRAIN_EXTRACT);
            UNIFORM(GRAIN_MERGE);
        case GIMP_HUE_MODE:
        case GIMP_SATURATION_MODE:
        case GIMP_VALUE_MODE:
        case GIMP_COLOR_MODE:
        {
            static struct HSV hsvTop, hsvBot ;
            RGBtoHSV(top->pixels[i],&hsvTop);
            if( mode == GIMP_HUE_MODE && hsvTop.ch1 == hsvTop.ch3 )
                continue ;
            RGBtoHSV(bot->pixels[i],&hsvBot);
            if( mode == GIMP_VALUE_MODE ) {
                if( hsvBot.ch3 ) {
                    hsvBot.ch1 = (hsvBot.ch1*hsvTop.ch3 + hsvBot.ch3/2) / hsvBot.ch3;
                    hsvBot.ch2 = (hsvBot.ch2*hsvTop.ch3 + hsvBot.ch3/2) / hsvBot.ch3;
                    hsvBot.ch3 = hsvTop.ch3 ;
                } else {
                    hsvBot.ch1 = hsvBot.ch2 = hsvBot.ch3 = hsvTop.ch3 ;
                }
            } else {
                unsigned mfNum, mfDenom ;
                if( mode == GIMP_HUE_MODE || mode == GIMP_COLOR_MODE ) {
                    mfNum   = hsvTop.ch2-hsvTop.ch1 ;
                    mfDenom = hsvTop.ch3-hsvTop.ch1 ;
                    hsvBot.hue = hsvTop.hue ;
                } else {
                    mfNum   = hsvBot.ch2-hsvBot.ch1 ;
                    mfDenom = hsvBot.ch3-hsvBot.ch1 ;
                }
                if( mode == GIMP_SATURATION_MODE ) {
                    if( hsvTop.ch3 == 0 )
                        hsvBot.ch1 = hsvBot.ch3 ; /* Black has no saturation */
                    else
                        hsvBot.ch1 = (hsvTop.ch1*hsvBot.ch3 + hsvTop.ch3/2) / hsvTop.ch3;
                } else if( mode == GIMP_COLOR_MODE ) {
                    /* GIMP_COLOR_MODE works in HSL space instead of HSV. We must
             * transfer H and S, keeping the L = ch1+ch3 of the bottom pixel,
             * but the S we transfer works differently from the S in HSV.
             */
                    unsigned L = hsvTop.ch1 + hsvTop.ch3 ;
                    unsigned sNum = hsvTop.ch3 - hsvTop.ch1 ;
                    unsigned sDenom = L < 256 ? L : 510-L ;
                    if( sDenom == 0 ) sDenom = 1 ; /* sNum will be 0 */
                    L = hsvBot.ch1 + hsvBot.ch3 ;
                    if( L < 256 ) {
                        /* Ideally we want to compute L/2 * (1-sNum/sDenom)
               * But shuffle this a bit so we can use integer arithmetic.
               * The "-1" in the rounding prevents us from ending up with
               * ch1 > ch3.
               */
                        hsvBot.ch1 = (L*(sDenom-sNum)+sDenom-1)/(2*sDenom);
                        hsvBot.ch3 = L - hsvBot.ch1 ;
                    } else {
                        /* Here our goal is 255 - (510-L)/2 * (1-sNum/sDenom) */
                        hsvBot.ch3 = 255 - ((510-L)*(sDenom-sNum)+sDenom-1)/(2*sDenom);
                        hsvBot.ch1 = L - hsvBot.ch3 ;
                    }
                    assert(hsvBot.ch3 <= 255);
                    assert(hsvBot.ch3 >= hsvBot.ch1);
                }
                if( mfDenom == 0 )
                    hsvBot.ch2 = hsvBot.ch1 ;
                else
                    hsvBot.ch2 = hsvBot.ch1 +
                            (mfNum*(hsvBot.ch3-hsvBot.ch1) + mfDenom/2) / mfDenom ;
            }
            switch( hsvBot.hue ) {
#define HEXTANT(b,m,t) case HUE_ ## b ## _ ## m ## _ ## t : \
    b = hsvBot.ch1; m = hsvBot.ch2; t = hsvBot.ch3; break;
            HEXTANT(RED,GREEN,BLUE);
            HEXTANT(RED,BLUE,GREEN);
            HEXTANT(BLUE,RED,GREEN);
            HEXTANT(BLUE,GREEN,RED);
            HEXTANT(GREEN,BLUE,RED);
            HEXTANT(GREEN,RED,BLUE);
#undef HEXTANT
            default: {

                    FatalUnexpected("Hue hextant is %d", hsvBot.hue);
                    return XCF_ERROR;
                }
            }
            break ;
        }
        default:
            {
                FatalUnsupportedXCF(_("'%s' layer mode"),
                                _(showGimpLayerModeEffects(mode)));
                return XCF_ERROR;
            }
        }
        if( FULLALPHA(bot->pixels[i] & top->pixels[i]) )
            bot->pixels[i] = (bot->pixels[i] & (255 << ALPHA_SHIFT)) +
                    (RED << RED_SHIFT) +
                    (GREEN << GREEN_SHIFT) +
                    (BLUE << BLUE_SHIFT) ;
        else {
            rgba bp = bot->pixels[i] ;
            /* In a sane world, the alpha of the top pixel would simply be
       * used to interpolate linearly between the bottom pixel's base
       * color and the effect-computed color.
       * But no! What the Gimp actually does is empirically
       * described by the following (which borrows code from
       * composite_one() that makes no theoretical sense here):
       */
            unsigned tfrac = ALPHA(top->pixels[i]) ;
            if( !FULLALPHA(bp) ) {
                unsigned pseudotop = (tfrac < ALPHA(bp) ? tfrac : ALPHA(bp));
                unsigned alpha = 255 ^ scaletable[255-ALPHA(bp)][255-pseudotop] ;
                tfrac = (256*pseudotop - 1) / alpha ;
            }
            bot->pixels[i] = (bp & (255 << ALPHA_SHIFT)) +
                    ((rgba)scaletable[  tfrac  ][  RED                ] << RED_SHIFT  ) +
                    ((rgba)scaletable[  tfrac  ][  GREEN              ] << GREEN_SHIFT) +
                    ((rgba)scaletable[  tfrac  ][  BLUE               ] << BLUE_SHIFT ) +
                    ((rgba)scaletable[255^tfrac][255&(bp>>RED_SHIFT  )] << RED_SHIFT  ) +
                    ((rgba)scaletable[255^tfrac][255&(bp>>GREEN_SHIFT)] << GREEN_SHIFT) +
                    ((rgba)scaletable[255^tfrac][255&(bp>>BLUE_SHIFT )] << BLUE_SHIFT ) ;
        }
    }
    return XCF_OK;
}

static void
dissolveTile(struct Tile *tile)
{
    unsigned i ;
    summary_t summary ;
    assert( tile->refcount == 1 );
    if( (tile->summary & TILESUMMARY_CRISP) )
        return ;
    summary = TILESUMMARY_UPTODATE + TILESUMMARY_ALLNULL
            + TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
    for( i = 0 ; i < tile->count ; i++ ) {
        if( FULLALPHA(tile->pixels[i]) )
            summary &= ~TILESUMMARY_ALLNULL ;
        else if ( NULLALPHA(tile->pixels[i]) )
            summary &= ~TILESUMMARY_ALLFULL ;
        else if( ALPHA(tile->pixels[i]) > rand() % 0xFF ) {
            tile->pixels[i] |= 255 << ALPHA_SHIFT ;
            summary &= ~TILESUMMARY_ALLNULL ;
        } else {
            tile->pixels[i] = 0 ;
            summary &= ~TILESUMMARY_ALLFULL ;
        }
    }
    tile->summary = summary ;
}

static void
roundAlpha(struct Tile *tile)
{
    unsigned i ;
    summary_t summary ;
    assert( tile->refcount == 1 );
    if( (tile->summary & TILESUMMARY_CRISP) )
        return ;
    summary = TILESUMMARY_UPTODATE + TILESUMMARY_ALLNULL
            + TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
    for( i = 0 ; i < tile->count ; i++ ) {
        if( ALPHA(tile->pixels[i]) >= 128 ) {
            tile->pixels[i] |= 255 << ALPHA_SHIFT ;
            summary &= ~TILESUMMARY_ALLNULL ;
        } else {
            tile->pixels[i] = 0 ;
            summary &= ~TILESUMMARY_ALLFULL ;
        }
    }
    tile->summary = summary ;
}

/* flattenTopdown() shares ownership of top.
 * The return value may be a shared tile.
 */
static struct Tile *
        flattenTopdown(struct FlattenSpec *spec, struct Tile *top,
                       unsigned nlayers, const struct rect *where)
{
    struct Tile *tile = 0;

    while( nlayers-- ) {
        if( tileSummary(top) & TILESUMMARY_ALLFULL ) {
            freeTile(tile);
            return top ;
        }
        if( !spec->layers[nlayers].isVisible )
            continue ;

        tile = getLayerTile(&spec->layers[nlayers],where);
        if (tile == XCF_PTR_EMPTY) {
            return XCF_PTR_EMPTY;
        }

        if( tile->summary & TILESUMMARY_ALLNULL )
            continue ; /* Simulate a tail call */

        switch( spec->layers[nlayers].mode ) {
        case GIMP_NORMAL_NOPARTIAL_MODE:
            roundAlpha(tile) ;
            /* Falls through */
        case GIMP_DISSOLVE_MODE:
            dissolveTile(tile);
            /* Falls through */
        case GIMP_NORMAL_MODE:
            top = merge_normal(tile,top);
            break ;
        default:
        {
            struct Tile *below, *above ;
            unsigned i ;
            if( !(top->summary & TILESUMMARY_ALLNULL) ) {
                rgba tile_or = 0 ;
                invalidateSummary(tile,0);
                for( i=0; i<top->count; i++ )
                    if( FULLALPHA(top->pixels[i]) )
                        tile->pixels[i] = 0 ;
                    else
                        tile_or |= tile->pixels[i] ;
                /* If the tile only has pixels that will be covered by 'top' anyway,
           * forget it anyway.
           */
                if( ALPHA(tile_or) == 0 ) {
                    freeTile(tile);
                    break ; /* from the switch, which will continue the while */
                }
            }
            /* Create a dummy top for the layers below this */
            if( top->summary & TILESUMMARY_CRISP ) {
                above = forkTile(top);
                if(above == XCF_PTR_EMPTY) {
                    return XCF_PTR_EMPTY;
                }
            } else {
                summary_t summary = TILESUMMARY_ALLNULL ;
                above = newTile(*where);
                for( i=0; i<top->count; i++ )
                    if( FULLALPHA(top->pixels[i]) ) {
                        above->pixels[i] = -1 ;
                        summary = 0 ;
                    } else
                        above->pixels[i] = 0 ;
                above->summary = TILESUMMARY_UPTODATE + TILESUMMARY_CRISP + summary;
            }
            below = flattenTopdown(spec, above, nlayers, where);
            if (below == XCF_PTR_EMPTY) {
                return XCF_PTR_EMPTY;
            }
            if( below->refcount > 1 ) {
                if (below != top) {
                    return XCF_PTR_EMPTY;
                }
                /* This can only happen if 'below' is a copy of 'top'
           * THROUGH 'above', which in turn means that none of all
           * this is visible after all. So just free it and return 'top'.
           */
                freeTile(below);
                return top ;
            }
            if (merge_exotic(below,tile,spec->layers[nlayers].mode) != XCF_OK) {
                return XCF_PTR_EMPTY;
            }
            freeTile(tile);
            top = merge_normal(below,top);
            return top ;
        }
        }
    }
    return top ;
}

static int
addBackground(struct FlattenSpec *spec, struct Tile *tile, unsigned ncols)
{
    unsigned i ;

    if( tileSummary(tile) & TILESUMMARY_ALLFULL )
        return XCF_OK;

    switch( spec->partial_transparency_mode ) {
    case FORBID_PARTIAL_TRANSPARENCY:
        if( !(tileSummary(tile) & TILESUMMARY_CRISP) ) {
            FatalGeneric(102,_("Flattened image has partially transparent pixels"));
            return XCF_ERROR;
        }
        break ;
    case DISSOLVE_PARTIAL_TRANSPARENCY:
        dissolveTile(tile);
        break ;
    case ALLOW_PARTIAL_TRANSPARENCY:
    case PARTIAL_TRANSPARENCY_IMPOSSIBLE:
        break ;
    }

    if( spec->default_pixel == CHECKERED_BACKGROUND ) {
        INIT_SCALETABLE_IF( !(tile->summary & TILESUMMARY_CRISP ) );
        for( i=0; i<tile->count; i++ )
            if( !FULLALPHA(tile->pixels[i]) ) {
                rgba fillwith = ((i/ncols)^(i%ncols))&8 ? 0x66 : 0x99 ;
                fillwith = graytable[fillwith] + (255 << ALPHA_SHIFT) ;
                if( NULLALPHA(tile->pixels[i]) )
                    tile->pixels[i] = fillwith ;
                else
                    tile->pixels[i] = composite_one(fillwith,tile->pixels[i]);
            }
        tile->summary = TILESUMMARY_UPTODATE +
                TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
        return XCF_OK;
    }
    if( !FULLALPHA(spec->default_pixel) )  return XCF_OK;
    if( tileSummary(tile) & TILESUMMARY_ALLNULL ) {
        fillTile(tile,spec->default_pixel);
    } else {
        INIT_SCALETABLE_IF( !(tile->summary & TILESUMMARY_CRISP) );
        for( i=0; i<tile->count; i++ )
            if( NULLALPHA(tile->pixels[i]) )
                tile->pixels[i] = spec->default_pixel ;
            else if( FULLALPHA(tile->pixels[i]) )
                ;
            else
                tile->pixels[i] = composite_one(spec->default_pixel,tile->pixels[i]);

        tile->summary = TILESUMMARY_UPTODATE +
                TILESUMMARY_ALLFULL + TILESUMMARY_CRISP ;
    }
    return XCF_OK;
}

int
flattenIncrementally(struct FlattenSpec *spec,lineCallback callback)
{
    rgba *rows[TILE_HEIGHT] ;
    unsigned i, y, nrows, ncols ;
    struct rect where ;
    struct Tile *tile ;
    static struct Tile toptile ;

    toptile.count = TILE_HEIGHT * TILE_WIDTH ;
    fillTile(&toptile,0);

    for( where.t = spec->dim.c.t; where.t < spec->dim.c.b; where.t=where.b ) {
        where.b = TILE_TOP(where.t)+TILE_HEIGHT ;
        if( where.b > spec->dim.c.b ) where.b = spec->dim.c.b ;
        nrows = where.b - where.t ;
        for( y = 0; y < nrows ; y++ )
            rows[y] = xcfmalloc(4*(spec->dim.c.r-spec->dim.c.l));

        for( where.l = spec->dim.c.l; where.l < spec->dim.c.r; where.l=where.r ) {
            where.r = TILE_LEFT(where.l)+TILE_WIDTH ;
            if( where.r > spec->dim.c.r ) where.r = spec->dim.c.r ;
            ncols = where.r - where.l ;

            toptile.count = ncols * nrows ;
            toptile.refcount = 2 ; /* For bug checking */
            assert( toptile.summary == TILESUMMARY_UPTODATE +
                    TILESUMMARY_ALLNULL + TILESUMMARY_CRISP );
            tile = flattenTopdown(spec,&toptile,spec->numLayers,&where) ;
            if (tile == XCF_PTR_EMPTY) {
                return XCF_ERROR;
            }
            toptile.refcount-- ; /* addBackground may change destructively */
            if (addBackground(spec,tile,ncols) != XCF_OK) {
                return XCF_ERROR;
            }

            for( i = 0 ; i < tile->count ; i++ )
                if( NULLALPHA(tile->pixels[i]) )
                    tile->pixels[i] = 0 ;
            for( y = 0 ; y < nrows ; y++ )
                memcpy(rows[y] + (where.l - spec->dim.c.l),
                       tile->pixels + y * ncols, ncols*4);

            if( tile == &toptile ) {
                fillTile(&toptile,0);
            } else {
                freeTile(tile);
            }
        }
        for( y = 0 ; y < nrows ; y++ )
            callback(spec->dim.width,rows[y]);
    }
    return XCF_OK;
}

static rgba **collectPointer ;

static void
collector(unsigned num,rgba *row)
{
    num += 0;
    *collectPointer++ = row ;
}

rgba **
flattenAll(struct FlattenSpec *spec)
{
    rgba **rows = xcfmalloc(spec->dim.height * sizeof(rgba*));
    if( verboseFlag )
        fprintf(stderr,_("Flattening image ..."));
    collectPointer = rows ;
    if (flattenIncrementally(spec,collector) != XCF_OK) {
        xcffree(rows);
        collectPointer = XCF_PTR_EMPTY;
        return XCF_PTR_EMPTY;
    }
    if( verboseFlag )
        fprintf(stderr,"\n");
    return rows ;
}

void
shipoutWithCallback(struct FlattenSpec *spec, rgba **pixels,
                    lineCallback callback)
{
    unsigned i ;
    for( i = 0; i < spec->dim.height; i++ ) {
        callback(spec->dim.width,pixels[i]);
    }
    xcffree(pixels);
}
