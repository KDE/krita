/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#ifndef INDEXCOLORPALETTE_H
#define INDEXCOLORPALETTE_H

#include <QVector>
#include <QColor>
#include <QPair>
#include <KoColor.h>

struct LabColor
{
    quint16 L;
    quint16 a;
    quint16 b;
};

struct IndexColorPalette
{
    QVector<LabColor> colors;

    struct
    {
        float L;
        float a;
        float b;
    } similarityFactors;

    IndexColorPalette();
    void insertShades(QColor clrA, QColor clrB, int shades);
    void insertShades(KoColor clrA, KoColor clrB, int shades);
    void insertShades(LabColor clrA, LabColor clrB, int shades);
    
    void insertColor(QColor clr);
    void insertColor(KoColor clr);
    void insertColor(LabColor clr);
    
    void mergeMostReduantColors();
    
    LabColor getNearestIndex(LabColor clr) const;
    int numColors() const;
    float similarity(LabColor c0, LabColor c1) const;
    QPair< int, int > getNeighbours(int mainClr) const;
};

#endif // INDEXCOLORPALETTE_H
