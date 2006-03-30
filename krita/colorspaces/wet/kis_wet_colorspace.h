/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_WET_H_
#define KIS_STRATEGY_COLORSPACE_WET_H_

#include <qcolor.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qmap.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"

class KisFilter;

/**
 * The wet colourspace is one of the more complicated colour spaces. Every
 * pixel actually consists of two pixels: the paint pixel and the adsorbtion
 * pixel. This corresponds to the two layers of the wetpack structure in the
 * original wetdreams code by Raph Levien.
 */

// XXX: This should really be in a namespace.

typedef struct _WetPix WetPix;
typedef struct _WetPixDbl WetPixDbl;
typedef struct _WetPack WetPack;

/*
    * White is made up of myth-red, myth-green, and myth-blue. Myth-red
    * looks red when viewed reflectively, but cyan when viewed
    * transmissively (thus, it vaguely resembles a dichroic
    * filter). Myth-red over black is red, and myth-red over white is
    * white.
    *
    * Total red channel concentration is myth-red concentration plus
    * cyan concentration.
    */

struct _WetPix {
    Q_UINT16 rd;  /*  Total red channel concentration */
    Q_UINT16 rw;  /*  Myth-red concentration */

    Q_UINT16 gd;  /*  Total green channel concentration */
    Q_UINT16 gw;  /*  Myth-green concentration */

    Q_UINT16 bd;  /*  Total blue channel concentration */
    Q_UINT16 bw;  /*  Myth-blue concentration */

    Q_UINT16 w;   /*  Water volume */
    Q_UINT16 h;   /*  Height of paper surface XXX: This might just as well be a single
                      channel in our colour model that has two of
                      these wetpix structs for every paint device pixels*/
};

struct _WetPack {
    WetPix paint;  /* Paint layer */
    WetPix adsorb; /* Adsorbtion layer */
};

struct _WetPixDbl {
    double rd;  /*  Total red channel concentration */
    double rw;  /*  Myth-red concentration */
    double gd;  /*  Total green channel concentration */
    double gw;  /*  Myth-green concentration */
    double bd;  /*  Total blue channel concentration */
    double bw;  /*  Myth-blue concentration */
    double w;   /*  Water volume */
    double h;   /*  Height of paper surface */
};



void wetPixToDouble(WetPixDbl * dst, WetPix *src);
void wetPixFromDouble(WetPix * dst, WetPixDbl *src);


class KisWetColorSpace : public KisAbstractColorSpace {
public:
    KisWetColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisWetColorSpace();


    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8 || independence == TO_LAB16)
                return true;
            else
                return false;
        };




public:

    // Semi-clever: we have only fifteen wet paint colors that are mapped to the
    // qcolors that are put in the painter by the special wet paint palette. Other
    // QColors are mapped to plain water...
    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile * profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile * profile = 0);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile * profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile * profile = 0);

    virtual Q_UINT8 getAlpha(const Q_UINT8 * pixel) const;
    virtual void setAlpha( Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels) const;
    virtual void multiplyAlpha( Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels);

    virtual void applyAlphaU8Mask( Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);
    virtual void applyInverseAlphaU8Mask( Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);

    virtual Q_UINT8 scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos);
    virtual Q_UINT16 scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos);

    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights,  Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual  Q_UINT32 nChannels() const;
    virtual  Q_UINT32 nColorChannels() const;
    virtual  Q_UINT32 nSubstanceChannels() const;
    virtual  Q_UINT32 pixelSize() const;

    virtual QString channelValueText(const Q_UINT8 *pixel,  Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel,  Q_UINT32 channelIndex) const;

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfile *  dstProfile,
                       Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
                       float exposure = 0.0f);

    //virtual QValueList<KisFilter*> createBackgroundFilters();
    
    virtual KisCompositeOpList userVisiblecompositeOps() const;

    void setPaintWetness(bool b) { m_paintwetness = b; } // XXX this needs better design!
    bool paintWetness() { return m_paintwetness; }
    void resetPhase() { phase = phasebig++; phasebig &= 3; }

    void combinePixels(WetPix* dst, WetPix const* src1, WetPix const* src2) const {
        dst->rd = src1->rd + src2->rd;
        dst->rw = src1->rw + src2->rw;
        dst->gd = src1->gd + src2->gd;
        dst->gw = src1->gw + src2->gw;
        dst->bd = src1->bd + src2->bd;
        dst->bw = src1->bw + src2->bw;
        dst->w = src1->w + src2->w;
    }
protected:
    virtual void bitBlt( Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowStride,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_UINT8 opacity,
            Q_INT32 rows,
            Q_INT32 cols,
            const KisCompositeOp& op);
private:

    // This was static, but since we have only one instance of the color strategy,
    // it can be just as well a private member variable.
    void wet_init_render_tab();

    /// Convert a single pixel from its wet representation to rgb: internal rgb: rgb[0] = R, etc
    typedef enum { RGB, BGR } RGBMode;
    void wet_composite(RGBMode m, Q_UINT8 *rgb, WetPix * wet);

    void wet_render_wetness( Q_UINT8 * rgb, WetPack * pack);

private:
     Q_UINT32 * wet_render_tab;

    QStringList m_paintNames;
    QMap<int, WetPix> m_conversionMap;

    bool m_paintwetness;
    int phase, phasebig;

};

class KisWetColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("WET", i18n("Watercolors")); };

    /**
     * lcms colorspace type definition.
     */
    virtual  Q_UINT32 colorSpaceType() { return 0; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icMaxEnumData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisWetColorSpace(parent, p); };

    virtual QString defaultProfile() { return ""; };
};

#endif // KIS_STRATEGY_COLORSPACE_WET_H_
