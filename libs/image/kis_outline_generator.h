/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007, 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  Outline algorithm based of the limn of fontutils
 *  SPDX-FileCopyrightText: 1992 Karl Berry <karl@cs.umb.edu>
 *  SPDX-FileCopyrightText: 1992 Kathryn Hargreaves <letters@cs.umb.edu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_OUTLINE_GENERATOR_H
#define KIS_OUTLINE_GENERATOR_H

#include <QPolygon>

#include "kritaimage_export.h"
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


private:

    enum EdgeType {
        TopEdge = 1, LeftEdge = 2, BottomEdge = 3, RightEdge = 0, NoEdge = 4
    };

    template <class StorageStrategy>
    QVector<QPolygon> outlineImpl(typename StorageStrategy::StorageType buffer,
                                  qint32 xOffset, qint32 yOffset,
                                  qint32 width, qint32 height);

    template <class StorageStrategy>
    bool isOutlineEdge(StorageStrategy &storage, EdgeType edge, qint32 x, qint32 y, qint32 bufWidth, qint32 bufHeight);

    template <class StorageStrategy>
    void nextOutlineEdge(StorageStrategy &storage, EdgeType *edge, qint32 *row, qint32 *col, qint32 width, qint32 height);

    EdgeType nextEdge(EdgeType edge) {
        return edge == NoEdge ? edge : static_cast<EdgeType>((edge + 1) % 4);
    }

    void appendCoordinate(QPolygon * path, int x, int y, EdgeType edge, EdgeType prevEdge);

private:

    const KoColorSpace* m_cs;
    quint8 m_defaultOpacity;
    bool m_simple;

    KisRandomConstAccessorSP m_accessor;
};

#endif // KIS_OUTLINE_GENERATOR_H
