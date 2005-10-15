/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <limits.h>
#include <stdlib.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_abstract_colorspace.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_image.h"
#include "kis_wet_colorspace.h"
#include "kis_integer_maths.h"

namespace {
    static const WetPix m_paint = { 707, 0, 707, 0, 707, 0, 240, 0 };

    /* colors from Curtis et al, Siggraph 97 */

    static const WetPix m_paintbox[] = {
        {496, 0, 16992, 0, 3808, 0, 0, 0},
        {16992, 9744, 21712, 6400, 25024, 3296, 0, 0},
        {6512, 6512, 6512, 4880, 11312, 0, 0, 0},
        {16002, 0, 2848, 0, 16992, 0, 0, 0},
        {22672, 0, 5328, 2272, 4288, 2640, 0, 0},
        {8000, 0, 16992, 0, 28352, 0, 0, 0},
        {5696, 5696, 12416, 2496, 28352, 0, 0, 0},
        {0, 0, 5136, 0, 28352, 0, 0, 0},
        {2320, 1760, 7344, 4656, 28352, 0, 0, 0},
        {8000, 0, 3312, 0, 5504, 0, 0, 0},
        {13680, 0, 16992, 0, 3312, 0, 0, 0},
        {5264, 5136, 1056, 544, 6448, 6304, 0, 0},
        {11440, 11440, 11440, 11440, 11440, 11440, 0, 0},
        {11312, 0, 11312, 0, 11312, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0} };

    static const int m_nPaints = 15;
}

void wetPixToDouble(WetPixDbl * dst, WetPix *src)
{
    dst->rd = (1.0 / 8192.0) * src->rd;
    dst->rw = (1.0 / 8192.0) * src->rw;
    dst->gd = (1.0 / 8192.0) * src->gd;
    dst->gw = (1.0 / 8192.0) * src->gw;
    dst->bd = (1.0 / 8192.0) * src->bd;
    dst->bw = (1.0 / 8192.0) * src->bw;
    dst->w = (1.0 / 8192.0) * src->w;
    dst->h = (1.0 / 8192.0) * src->h;
}

void wetPixFromDouble(WetPix * dst, WetPixDbl *src)
{
    int v;

    v = floor (8192.0 * src->rd + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->rd = v;

    v = floor (8192.0 * src->rw + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->rw = v;

    v = floor (8192.0 * src->gd + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->gd = v;

    v = floor (8192.0 * src->gw + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->gw = v;

    v = floor (8192.0 * src->bd + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->bd = v;

    v = floor (8192.0 * src->bw + 0.5);
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst->bw = v;

    v = floor (8192.0 * src->w + 0.5);
    if (v < 0) v = 0;
    if (v > 511) v = 511;
    dst->w = v;

    v = floor (8192.0 * src->h + 0.5);
    if (v < 0) v = 0;
    if (v > 511) v = 511;
    dst->h = v;

}

KisWetColorSpace::KisWetColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisAbstractColorSpace(KisID("WET", i18n("Watercolors")), 0, icMaxEnumData, parent, p)
{
    wet_init_render_tab();

    m_paintNames << i18n("Quinacridone Rose")
        << i18n("Indian Red")
        << i18n("Cadmium Yellow")
        << i18n("Hookers Green")
        << i18n("Cerulean Blue")
        << i18n("Burnt Umber")
        << i18n("Cadmium Red")
        << i18n("Brilliant Orange")
        << i18n("Hansa Yellow")
        << i18n("Phthalo Green")
        << i18n("French Ultramarine")
        << i18n("Interference Lilac")
        << i18n("Titanium White")
        << i18n("Ivory Black")
        << i18n("Pure Water");

    m_channels.push_back(new KisChannelInfo(i18n("Red Concentration"), 0, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Myth Red"), 1, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Green Concentration"), 2, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Myth Green"), 3, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Blue Concentration"), 4, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Myth Blue"), 5, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Water Volume"), 6, SUBSTANCE));
    m_channels.push_back(new KisChannelInfo(i18n("Paper Height"), 7, SUBSTANCE));

    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Red Concentration"), 8, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Myth Red"), 9, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Green Concentration"), 10, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Myth Green"), 11, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Blue Concentration"), 12, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Myth Blue"), 13, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Water Volume"), 14, SUBSTANCE));
    m_channels.push_back(new KisChannelInfo(i18n("Adsorbed Paper Height"), 15, SUBSTANCE));


    // we store the conversion in an QRgb (equivalent to unsigned int)
    // since QColor does not provide an operator<
    m_conversionMap[qRgb(240, 32, 160)] = m_paintbox[0]; // Quinacridone Rose
    m_conversionMap[qRgb(159, 88, 43)] = m_paintbox[1]; // Indian Red
    m_conversionMap[qRgb(254, 220, 64)] = m_paintbox[2]; // Cadmium Yellow
    m_conversionMap[qRgb(36, 180, 32)] = m_paintbox[3]; // Hookers Green
    m_conversionMap[qRgb(16, 185, 215)] = m_paintbox[4]; // Cerulean Blue
    m_conversionMap[qRgb(96, 32, 8)] = m_paintbox[5]; // Burnt Umber
    m_conversionMap[qRgb(254, 96, 8)] = m_paintbox[6]; // Cadmium Red
    m_conversionMap[qRgb(255, 136, 8)] = m_paintbox[7]; // Brilliant Orange
    m_conversionMap[qRgb(240, 199, 8)] = m_paintbox[8]; // Hansa Yellow
    m_conversionMap[qRgb(96, 170, 130)] = m_paintbox[9]; // Phthalo Green
    m_conversionMap[qRgb(48, 32, 170)] = m_paintbox[10]; // French Ultramarine
    m_conversionMap[qRgb(118, 16, 135)] = m_paintbox[11]; // Interference Lilac
    m_conversionMap[qRgb(254, 254, 254)] = m_paintbox[12]; // Titanium White
    m_conversionMap[qRgb(64, 64, 74)] = m_paintbox[13]; // Ivory Black

    m_paintwetness = false;
    phasebig = 0;
}


KisWetColorSpace::~KisWetColorSpace()
{
}

void KisWetColorSpace::fromQColor(const QColor& c, Q_UINT8 *dst)
{
    WetPack* p = reinterpret_cast<WetPack*>(dst);

    // Translate the special QCOlors from our paintbox to wetpaint paints.
    // XXX: Define a class that combines QColor, wet paint color and name.
    if (m_conversionMap.contains(c.rgb())) {
        (*p).paint = m_conversionMap[c.rgb()];
        (*p).adsorb = m_conversionMap[c.rgb()]; // or maybe best add water here?
    } else {
        kdDebug(DBG_AREA_CMS) << c.red() << " " << c.green() << " " << c.blue() << endl;
        // water
        (*p).paint = m_paintbox[14];
        (*p).adsorb = m_paintbox[14];
    }

    // XXX: Maybe somehow do something useful with QColor that don't correspond to paint from the paintbox.
}

void KisWetColorSpace::fromQColor(const QColor& c, Q_UINT8  /*opacity*/, Q_UINT8 *dst)
{
    fromQColor(c, dst);
}

Q_UINT8 KisWetColorSpace::getAlpha(const Q_UINT8 */*pixel*/)
{
    return OPACITY_OPAQUE;
}

void KisWetColorSpace::setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{
}

void KisWetColorSpace::applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
}

void KisWetColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
}

Q_UINT8 KisWetColorSpace::scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos)
{
    return 0;
}

Q_UINT16 KisWetColorSpace::scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos)
{
    return 0;
}


void KisWetColorSpace::toQColor(const Q_UINT8 *src, QColor *c)
{
    Q_UINT8 * rgb = new Q_UINT8[3];
    Q_CHECK_PTR(rgb);

    memset(rgb, 255, 3);

    // Composite the two layers in each pixelSize

    WetPack * wp = (WetPack*)src;

    // First the adsorption layers
    wet_composite(rgb, &wp -> adsorb);

    // Then the paint layer (which comes first in our double-packed pixel)
    wet_composite(rgb,  &wp -> paint);

    c -> setRgb(rgb[0], rgb[1], rgb[2]);

    delete[]rgb;
}

void KisWetColorSpace::toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity)
{
    toQColor(src, c);
}

void KisWetColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

QValueVector<KisChannelInfo *> KisWetColorSpace::channels() const
{
    return m_channels;
}

bool KisWetColorSpace::hasAlpha() const
{
    return false;
}

Q_INT32 KisWetColorSpace::nChannels() const
{
    return 16;
}

Q_INT32 KisWetColorSpace::nColorChannels() const
{
    return 12;
}

Q_INT32 KisWetColorSpace::nSubstanceChannels() const
{
    return 4;
}


Q_INT32 KisWetColorSpace::pixelSize() const
{
    return 32; // This color strategy wants an unsigned short for each
           // channel, and every pixel consists of two wetpix structs
           // -- even though for many purposes we need only one wetpix
           // struct.
}



// XXX: use profiles to display correctly on calibrated displays.
QImage KisWetColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                         KisProfile *  /*dstProfile*/,
                         Q_INT32 /*renderingIntent*/, float /*exposure*/)
{

    QImage img(width, height, 32);

    Q_UINT8 *rgb = (Q_UINT8*) img.bits();
    const WetPack* wetData = reinterpret_cast<const WetPack*>(data);

    // Clear to white -- the following code actually composits the contents of the
    // wet pixels with the contents of the image buffer, so they need to be
    // prepared
    memset(rgb, 255, width * height * 4);
    // Composite the two layers in each pixelSize

    Q_INT32 i = 0;
    while ( i < width * height) {
        // First the adsorption layers
        WetPack* wp = const_cast<WetPack*>(&wetData[i]); // XXX don't do these things!
        wet_composite(rgb, &(wp -> adsorb));

        // Then the paint layer (which comes first in our double-packed pixel)
        wet_composite(rgb, &(wp -> paint));

        // XXX pay attention to this comment!!
        // Display the wet stripes -- this only works if we have at least three scanlines in height,
        // because otherwise the phase trick won't work.

        // Because we work in a stateless thing, and we can't just draw this wetness
        // indication AFTER this (e.g. like the selection), we have to do un nice things:
        // Because we (hopefully atm!) don't use the height of the paint wetpix, we abuse
        // that to store a state. It's not perfect, but it works for now...
        if (m_paintwetness) {
            wet_render_wetness(rgb, wp);
        }

        i++;
        rgb += sizeof(Q_UINT32); // Because the QImage is 4 bytes deep.

    }

    return img;
}

void KisWetColorSpace::bitBlt(Q_UINT8 *dst,
                  Q_INT32 dstRowSize,
                  const Q_UINT8 *src,
                  Q_INT32 srcRowStride,
                  const Q_UINT8 */*srcAlphaMask*/,
                  Q_INT32 /*maskRowStride*/,
                  Q_UINT8 /*opacity*/,
                  Q_INT32 rows,
                  Q_INT32 cols,
                  const KisCompositeOp& /*op*/)
{
    if (rows <= 0 || cols <= 0)
        return;

    Q_UINT8 *d;
    const Q_UINT8 *s;

    // Just copy the src onto the dst, we don't do fancy things here,
    // we do those in the paint op, because we need pressure to determine
    // paint deposition.
    Q_INT32 linesize = pixelSize() * cols;
    d = dst;
    s = src;
    while (rows-- > 0) {
        memcpy(d, s, linesize);
        d += dstRowSize; // size??
        s += srcRowStride;
    }
}

void KisWetColorSpace::wet_init_render_tab()
{
    int i;

    double d;
    int a, b;

    wet_render_tab = new Q_UINT32[4096];
    Q_CHECK_PTR(wet_render_tab);

    for (i = 0; i < 4096; i++)
    {
        d = i * (1.0 / 512.0);

        if (i == 0)
            a = 0;
        else
            a = floor (0xff00 / i + 0.5);

        b = floor (0x8000 * exp (-d) + 0.5);
        wet_render_tab[i] = (a << 16) | b;
    }

}

void KisWetColorSpace::wet_composite(Q_UINT8 *rgb, WetPix * wet)
{
    int r, g, b;
    int d, w;
    int ab;
    int wa;

    r = rgb[0];
    w = wet[0].rw >> 4;
    d = wet[0].rd >> 4;


    ab = wet_render_tab[d];

    wa = (w * (ab >> 16) + 0x80) >> 8;
    r = wa + (((r - wa) * (ab & 0xffff) + 0x4000) >> 15);
    rgb[0] = r;

    g = rgb[1];
    w = wet[0].gw >> 4;
    d = wet[0].gd >> 4;
    d = d >= 4096 ? 4095 : d;
    ab = wet_render_tab[d];
    wa = (w * (ab >> 16) + 0x80) >> 8;
    g = wa + (((g - wa) * (ab & 0xffff) + 0x4000) >> 15);
    rgb[1] = g;

    b = rgb[2];
    w = wet[0].bw >> 4;
    d = wet[0].bd >> 4;
    d = d >= 4096 ? 4095 : d;
    ab = wet_render_tab[d];
    wa = (w * (ab >> 16) + 0x80) >> 8;
    b = wa + (((b - wa) * (ab & 0xffff) + 0x4000) >> 15);
    rgb[2] = b;

}

void KisWetColorSpace::wet_render_wetness(Q_UINT8 * rgb, WetPack * pack)
{
    int highlight = 255 - (pack -> paint.w >> 1);

    if (highlight < 255 && ((phase++) % 3 == 0)) {
        for (int i = 0; i < 3; i++)
            rgb[i] = 255 - (((255 - rgb[i]) * highlight) >> 8);
    }
    phase &= 3;
}

KisCompositeOpList KisWetColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}

QString KisWetColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisWetColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT16_MAX);
}

