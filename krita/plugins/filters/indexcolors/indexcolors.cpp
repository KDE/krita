/*
 * Copyright 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#include "indexcolors.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QTime>
#include <qmath.h>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_processing_information.h>
#include <kis_types.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>

#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSet.h>
#include <filter/kis_filter_configuration.h>
#include <widgets/kis_multi_integer_filter_widget.h>

#include "kiswdgindexcolors.h"
#include "palettegeneratorconfig.h"

K_PLUGIN_FACTORY(IndexColorsFactory, registerPlugin<IndexColors>();)
K_EXPORT_PLUGIN(IndexColorsFactory("krita"))

IndexColors::IndexColors(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterIndexColors()));
}

IndexColors::~IndexColors()
{
}

KisFilterIndexColors::KisFilterIndexColors() : KisColorTransformationFilter(id(), categoryArtistic(), i18n("&Index Colors..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT); // Technically it is TO_LAB16 but that would only display a warning we don't want
    // This filter will always degrade the color space, that is it's purpose
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KoColorTransformation* KisFilterIndexColors::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    KisIndexColorPalette pal;

    PaletteGeneratorConfig palCfg;
    palCfg.fromByteArray(config->getProperty("paletteGen").toByteArray());

    // Add all colors to the palette
    for(int y = 0; y < 4; ++y)
        for(int x = 0; x < 4; ++x)
            if(palCfg.colorsEnabled[y][x])
                pal.insertColor(palCfg.colors[y][x]);

    // Now all this convulved code is for determining what gradients should be generated
    for(int y = 0; y < 3; ++y)
        for(int x = 0; x < 4; ++x)
            if(palCfg.colorsEnabled[y][x] && palCfg.colorsEnabled[y+1][x])
                pal.insertShades(palCfg.colors[y][x], palCfg.colors[y+1][x], palCfg.gradientSteps[y]);

    if(palCfg.inbetweenRampSteps != 2)
    {
        for(int y = 0; y < 4; ++y)
            for(int x = 0; x < 3; ++x)
                if(palCfg.colorsEnabled[y][x] && palCfg.colorsEnabled[y][x+1])
                    pal.insertShades(palCfg.colors[y][x], palCfg.colors[y][x+1], palCfg.inbetweenRampSteps);
    }

    if(palCfg.diagonalGradients)
        for(int y = 0; y < 3; ++y)
            for(int x = 0; x < 4; ++x)
            {
                if(x+1 < 4)
                    if(palCfg.colorsEnabled[y][x+1] && palCfg.colorsEnabled[y+1][x])
                        pal.insertShades(palCfg.colors[y][x+1], palCfg.colors[y+1][x], palCfg.gradientSteps[y]);
                if(x-1 >= 0)
                    if(palCfg.colorsEnabled[y][x-1] && palCfg.colorsEnabled[y+1][x])
                        pal.insertShades(palCfg.colors[y][x-1], palCfg.colors[y+1][x], palCfg.gradientSteps[y]);
            }

    pal.similarityFactors.L = config->getFloat("LFactor");
    pal.similarityFactors.a = config->getFloat("aFactor");
    pal.similarityFactors.b = config->getFloat("bFactor");
    return new KisIndexColorTransformation(pal, cs, config->getInt("alphaSteps"));
}

KisConfigWidget* KisFilterIndexColors::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    KisWdgIndexColors* w = new KisWdgIndexColors(parent);
    w->setup(
        QStringList() << "Bright" << "Light"   << "Base" << "Shadow", 4
    );
    return w;
}

KisFilterConfiguration* KisFilterIndexColors::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 0);

    PaletteGeneratorConfig palCfg; // Default constructor is factory config
    config->setProperty("paletteGen",     palCfg.toByteArray());

    config->setProperty("LFactor",    1.f);
    config->setProperty("aFactor",    1.f);
    config->setProperty("bFactor",    1.f);
    config->setProperty("alphaSteps", 2);
    return config;
}

KisIndexColorTransformation::KisIndexColorTransformation(KisIndexColorPalette palette, const KoColorSpace* cs, int alphaSteps)
    : m_colorSpace(cs),
      m_psize(cs->pixelSize())
{
    m_palette = palette;

    static const qreal max = KoColorSpaceMathsTraits<quint16>::max;
    if(alphaSteps > 0)
    {
        m_alphaStep = max / alphaSteps;
        m_alphaHalfStep = m_alphaStep / 2;
    }
    else
    {
        m_alphaStep = 0;
        m_alphaHalfStep = 0;
    }
}

float KisIndexColorPalette::similarity(LabColor c0, LabColor c1) const
{
    static const qreal max = KoColorSpaceMathsTraits<quint16>::max;
    quint16 diffL = qAbs(c0.L - c1.L);
    quint16 diffa = qAbs(c0.a - c1.a);
    quint16 diffb = qAbs(c0.b - c1.b);
    float valL = diffL/max*similarityFactors.L;
    float valA = diffa/max*similarityFactors.a;
    float valB = diffb/max*similarityFactors.b;
    return 1.f - qSqrt(valL * valL + valA * valA + valB * valB);
}

LabColor KisIndexColorPalette::getNearestIndex(LabColor clr) const
{
    float diffs[256];
    for(int i = 0; i < numColors; ++i)
        diffs[i] = similarity(colors[i], clr);

    int primaryColor = 0;
    for(int i = 0; i < numColors; ++i)
        if(diffs[i] > diffs[primaryColor])
            primaryColor = i;

    return colors[primaryColor];
}

void KisIndexColorPalette::insertShades(LabColor clrA, LabColor clrB, int shades)
{
    if(shades == 0) return;
    qint16  lumaStep = (clrB.L - clrA.L) / shades;
    qint16 astarStep = (clrB.a - clrA.a) / shades;
    qint16 bstarStep = (clrB.b - clrA.b) / shades;
    for(int i = 0; i < shades; ++i)
    {
        clrA.L += lumaStep;
        clrA.a += astarStep;
        clrA.b += bstarStep;
        insertColor(clrA);
    }
}

void KisIndexColorPalette::insertShades(KoColor koclrA, KoColor koclrB, int shades)
{
    koclrA.convertTo(KoColorSpaceRegistry::instance()->lab16());
    koclrB.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clrA = *(reinterpret_cast<LabColor*>(koclrA.data()));
    LabColor clrB = *(reinterpret_cast<LabColor*>(koclrB.data()));
    insertShades(clrA, clrB, shades);
}

void KisIndexColorPalette::insertShades(QColor qclrA, QColor qclrB, int shades)
{
    KoColor koclrA;
    koclrA.fromQColor(qclrA);
    koclrA.convertTo(KoColorSpaceRegistry::instance()->lab16());
    KoColor koclrB;
    koclrB.fromQColor(qclrB);
    koclrB.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clrA = *(reinterpret_cast<LabColor*>(koclrA.data()));
    LabColor clrB = *(reinterpret_cast<LabColor*>(koclrB.data()));
    insertShades(clrA, clrB, shades);
}

void KisIndexColorPalette::insertColor(LabColor clr)
{
    if(numColors == 255) return;
    colors[numColors++] = clr;
}

void KisIndexColorPalette::insertColor(KoColor koclr)
{
    koclr.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clr = *(reinterpret_cast<LabColor*>(koclr.data()));
    insertColor(clr);
}

void KisIndexColorPalette::insertColor(QColor qclr)
{
    KoColor koclr;
    koclr.fromQColor(qclr);
    koclr.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clr = *(reinterpret_cast<LabColor*>(koclr.data()));
    insertColor(clr);
}


void KisIndexColorTransformation::transform(const quint8* src, quint8* dst, qint32 nPixels) const
{
    union
    {
        quint16 laba[4];
        LabColor lab;
    } clr;
    while (nPixels--)
    {
        m_colorSpace->toLabA16(src, reinterpret_cast<quint8 *>(clr.laba), 1);
        clr.lab = m_palette.getNearestIndex(clr.lab);
        if(m_alphaStep)
        {
            quint16 amod = clr.laba[3] % m_alphaStep;
            clr.laba[3] = clr.laba[3] + (amod > m_alphaHalfStep ? clr.laba[3] - amod : -amod);
        }
        m_colorSpace->fromLabA16(reinterpret_cast<quint8 *>(clr.laba), dst, 1);
        src += m_psize;
        dst += m_psize;
    }
}
