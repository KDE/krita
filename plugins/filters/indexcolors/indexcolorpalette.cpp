/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#include "indexcolorpalette.h"

#include <qmath.h>

#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <filter/kis_filter_configuration.h>
#include <widgets/kis_multi_integer_filter_widget.h>

float IndexColorPalette::similarity(LabColor c0, LabColor c1) const
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

IndexColorPalette::IndexColorPalette()
{
    similarityFactors.L = 1.0f;
    similarityFactors.a = 1.0f;
    similarityFactors.b = 1.0f;
}

int IndexColorPalette::numColors() const
{
    return colors.size();
}

LabColor IndexColorPalette::getNearestIndex(LabColor clr) const
{
    QVector<float> diffs;
    diffs.resize(numColors());
    for(int i = 0; i < numColors(); ++i)
        diffs[i] = similarity(colors[i], clr);

    int primaryColor = 0;
    for(int i = 0; i < numColors(); ++i)
        if(diffs[i] > diffs[primaryColor])
            primaryColor = i;

    return colors[primaryColor];
}

QPair<int, int> IndexColorPalette::getNeighbours(int mainClr) const
{
    QVector<float> diffs;
    diffs.resize(numColors());
    for(int i = 0; i < numColors(); ++i)
        diffs[i] = similarity(colors[i], colors[mainClr]);

    int darkerColor = 0;
    int brighterColor = 0;
    for(int i = 0; i < numColors(); ++i)
    {
        if(i != mainClr)
        {
            if(colors[i].L < colors[mainClr].L)
            {
                if(diffs[i] > diffs[darkerColor])
                    darkerColor = i;
            }
            else
            {
                if(diffs[i] > diffs[brighterColor])
                    brighterColor = i;
            }
        }
    }

    return qMakePair(darkerColor, brighterColor);
}

void IndexColorPalette::insertShades(LabColor clrA, LabColor clrB, int shades)
{
    if(shades == 0) return;
    qint16  lumaStep = (clrB.L - clrA.L) / (shades+1);
    qint16 astarStep = (clrB.a - clrA.a) / (shades+1);
    qint16 bstarStep = (clrB.b - clrA.b) / (shades+1);
    for(int i = 0; i < shades; ++i)
    {
        clrA.L += lumaStep;
        clrA.a += astarStep;
        clrA.b += bstarStep;
        insertColor(clrA);
    }
}

void IndexColorPalette::insertShades(KoColor koclrA, KoColor koclrB, int shades)
{
    koclrA.convertTo(KoColorSpaceRegistry::instance()->lab16());
    koclrB.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clrA = *(reinterpret_cast<LabColor*>(koclrA.data()));
    LabColor clrB = *(reinterpret_cast<LabColor*>(koclrB.data()));
    insertShades(clrA, clrB, shades);
}

void IndexColorPalette::insertShades(QColor qclrA, QColor qclrB, int shades)
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

void IndexColorPalette::insertColor(LabColor clr)
{
    colors.append(clr);
}

void IndexColorPalette::insertColor(KoColor koclr)
{
    koclr.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clr = *(reinterpret_cast<LabColor*>(koclr.data()));
    insertColor(clr);
}

void IndexColorPalette::insertColor(QColor qclr)
{
    KoColor koclr;
    koclr.fromQColor(qclr);
    koclr.convertTo(KoColorSpaceRegistry::instance()->lab16());
    LabColor clr = *(reinterpret_cast<LabColor*>(koclr.data()));
    insertColor(clr);
}

namespace
{
    struct ColorString
    {
        int color;
        QPair<int, int> neighbours;
        float similarity;
    };
}

void IndexColorPalette::mergeMostReduantColors()
{
    QVector<ColorString> colorHood;
    colorHood.resize(numColors());
    for(int i = 0; i < numColors(); ++i)
    {
        colorHood[i].color = i;
        colorHood[i].neighbours = getNeighbours(i);
        float lSimilarity = 0.05f, rSimilarity = 0.05f;
        // There will be exactly 2 colors that have only 1 neighbour, the darkest and the brightest, we don't want to remove those
        if(colorHood[i].neighbours.first  != -1)
            lSimilarity = similarity(colors[colorHood[i].neighbours.first], colors[i]);
        if(colorHood[i].neighbours.second != -1)
            rSimilarity = similarity(colors[colorHood[i].neighbours.second], colors[i]);
        colorHood[i].similarity = (lSimilarity + rSimilarity) / 2;
    }
    int mostSimilarColor = 0;
    for(int i = 0; i < numColors(); ++i)
        if(colorHood[i].similarity > colorHood[mostSimilarColor].similarity)
            mostSimilarColor = i;

    int darkerIndex = colorHood[mostSimilarColor].neighbours.first;
    int brighterIndex = colorHood[mostSimilarColor].neighbours.second;
    if(darkerIndex   != -1 &&
       brighterIndex != -1)
    {
        LabColor clrA = colors[darkerIndex];
        LabColor clrB = colors[mostSimilarColor];
        // Remove two, add one = 1 color less
        colors.remove(darkerIndex);
        colors.remove(mostSimilarColor);
        //colors.remove(brighterIndex);
        insertShades(clrA, clrB, 1);
        //insertShades(clrB, clrC, 1);
    }
}
