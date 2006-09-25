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

#include <QColor>
#include <QStringList>
#include <q3valuelist.h>
#include <QMap>

#include "KoLcmsColorSpaceTrait.h"

class KisFilter;

/**
 * The wet colorspace is one of the more complicated color spaces. Every
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
    quint16 rd;  /*  Total red channel concentration */
    quint16 rw;  /*  Myth-red concentration */

    quint16 gd;  /*  Total green channel concentration */
    quint16 gw;  /*  Myth-green concentration */

    quint16 bd;  /*  Total blue channel concentration */
    quint16 bw;  /*  Myth-blue concentration */

    quint16 w;   /*  Water volume */
    quint16 h;   /*  Height of paper surface XXX: This might just as well be a single
                      channel in our color model that has two of
                      these wetpix structs for every paint device pixels*/
};

struct _WetPack {
    WetPix paint;  /* Paint layer */
    WetPix adsorb; /* Adsorbtion layer */
    quint8 alpha;  /* for layer composition */
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


class KisWetColorSpace : public KoLcmsColorSpaceTrait {
public:
    KisWetColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
    virtual ~KisWetColorSpace();


    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8 || independence == TO_LAB16)
                return true;
            else
                return false;
        };




public:

    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0);

    virtual quint8 getAlpha(const quint8 * pixel) const;
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels);

    virtual void applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);

    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos);
    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos);

    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual Q3ValueVector<KoChannelInfo *> channels() const;
    virtual quint32 nChannels() const;
    virtual quint32 nColorChannels() const;
    virtual quint32 nSubstanceChannels() const;
    virtual quint32 pixelSize() const;

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                       KoColorProfile *  dstProfile,
                       qint32 renderingIntent = INTENT_PERCEPTUAL,
                       float exposure = 0.0f);

    virtual Q3ValueList<KisFilter*> createBackgroundFilters();
    
    virtual KoCompositeOpList userVisiblecompositeOps() const;

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
    virtual void bitBlt(quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowStride,
            const quint8 *srcAlphaMask,
            qint32 maskRowStride,
            quint8 opacity,
            qint32 rows,
            qint32 cols,
            const KoCompositeOp* op);
private:

    // This was static, but since we have only one instance of the color strategy,
    // it can be just as well a private member variable.
    void wet_init_render_tab();

    /// Convert a single pixel from its wet representation to rgb: internal rgb: rgb[0] = R, etc
    typedef enum { RGB, BGR } RGBMode;
    void wet_composite(RGBMode m, quint8 *rgb, WetPix * wet);

    void wet_render_wetness(quint8 * rgb, WetPack * pack);

private:
    quint32 * wet_render_tab;

    QStringList m_paintNames;
    QMap<int, WetPix> m_conversionMap;

    bool m_paintwetness;
    int phase, phasebig;

};

class KisWetColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("WET", i18n("Watercolors")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return 0; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icMaxEnumData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KisWetColorSpace(parent, p); };

    virtual QString defaultProfile() { return ""; };
};

#endif // KIS_STRATEGY_COLORSPACE_WET_H_
