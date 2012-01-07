/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007,2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  Outline algorithm based of the limn of fontutils
 *  Copyright (c) 1992 Karl Berry <karl@cs.umb.edu>
 *  Copyright (c) 1992 Kathryn Hargreaves <letters@cs.umb.edu>
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

#ifndef KIS_OUTLINE_GENERATOR_H
#define KIS_OUTLINE_GENERATOR_H

#include <QPolygon>

#include "krita_export.h"
#include "kis_types.h"
#include <kis_random_accessor_ng.h>
class KoColorSpace;


/**
 * Generates an 'outline' for a paint device. Used e.g. in for brushes and marching ants
 **/
class KRITAIMAGE_EXPORT KisOutlineGenerator
{
public:

    /**
     * Create an outline generator
     * @param cs colorspace for the buffer passed to the generator
     * @param defaultOpacity opacity of pixels that shouldn't be included in the outline
     **/
    KisOutlineGenerator(const KoColorSpace* cs, quint8 defaultOpacity);
    
    /**
     * Generates the outline.
     * @param buffer buffer with the data for the outline
     * @param xOffset offset that will be used for the x coordinate of the polygon points
     * @param yOffset offset that will be used for the y coordinate of the polygon points
     * @param width width of the buffer
     * @param height height of the buffer
     * @returns list of polygons around every non-transparent area
     **/
    QVector<QPolygon> outline(quint8* buffer, qint32 xOffset, qint32 yOffset, qint32 width, qint32 height);

    QVector<QPolygon> outline(const KisPaintDevice *buffer, qint32 xOffset, qint32 yOffset, qint32 width, qint32 height);


    /**
     * Set the generator to produce simpile outline, skipping outline that are fully enclosed
     * @param simple set simple mode, if true enclosed outline will be skipped
     **/
    void setSimpleOutline(bool simple);

private:

    enum EdgeType {
        TopEdge = 1, LeftEdge = 2, BottomEdge = 3, RightEdge = 0, NoEdge = 4
    };

    bool isOutlineEdge(EdgeType edge, qint32 x, qint32 y, quint8* buffer, qint32 bufWidth, qint32 bufHeight);
    bool isOutlineEdge(EdgeType edge, qint32 x, qint32 y, const KisPaintDevice *buffer, qint32 bufWidth, qint32 bufHeight);

    EdgeType nextEdge(EdgeType edge) {
        return edge == NoEdge ? edge : static_cast<EdgeType>((edge + 1) % 4);
    }

    void nextOutlineEdge(EdgeType *edge, qint32 *row, qint32 *col, quint8* buffer, qint32 bufWidth, qint32 bufHeight);
    void nextOutlineEdge(EdgeType *edge, qint32 *row, qint32 *col, const KisPaintDevice *buffer, qint32 bufWidth, qint32 bufHeight);

    void appendCoordinate(QPolygon * path, int x, int y, EdgeType edge);

    const KoColorSpace* m_cs;
    quint8 m_defaultOpacity;
    bool m_simple;

    qint32 m_xOffset;
    qint32 m_yOffset;

    KisRandomConstAccessorSP m_accessor;
};

#endif // KIS_OUTLINE_GENERATOR_H
