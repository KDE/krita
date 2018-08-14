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
