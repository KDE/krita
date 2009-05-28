/*
 *  kis_tool_moutline.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_CURVE_MAGNETIC_H_
#define KIS_CURVE_MAGNETIC_H_

#include "kis_curve_framework.h"
#include <Q3ValueVector>
#include <QRect>

#include <kis_types.h>

class QSlider;
class KisToolMagnetic;
class Node;

typedef Q3ValueVector<Node> NodeCol;
typedef Q3ValueVector<NodeCol> NodeMatrix;
typedef Q3ValueVector<qint16> GrayCol;
typedef Q3ValueVector<GrayCol> GrayMatrix;

class KisCurveMagnetic : public KisCurve
{

    KisToolMagnetic *m_parent;

    void reduceMatrix(QRect&, GrayMatrix&, int, int, int, int);
    void findEdge(int, int, const GrayMatrix&, Node&);
    void detectEdges(const QRect&, KisPaintDeviceSP, GrayMatrix&);

    void gaussianBlur(const QRect&, KisPaintDeviceSP, KisPaintDeviceSP);
    void toGrayScale(const QRect&, KisPaintDeviceSP, GrayMatrix&);
    void getDeltas(const GrayMatrix&, GrayMatrix&, GrayMatrix&);
    void getMagnitude(const GrayMatrix&, const GrayMatrix&, GrayMatrix&);
    void nonMaxSupp(const GrayMatrix&, const GrayMatrix&, const GrayMatrix&, GrayMatrix&);

public:

    KisCurveMagnetic(KisToolMagnetic *parent);
    ~KisCurveMagnetic();

    virtual KisCurve::iterator addPivot(iterator, const QPointF&);
    virtual KisCurve::iterator pushPivot(const QPointF&);
    virtual void calculateCurve(iterator, iterator, iterator);

};

#endif
