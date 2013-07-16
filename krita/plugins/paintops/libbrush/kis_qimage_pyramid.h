/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_QIMAGE_PYRAMID_H
#define __KIS_QIMAGE_PYRAMID_H

#include <QImage>
#include <QVector>


class KisQImagePyramid
{
public:
    KisQImagePyramid(const QImage &baseImage);
    ~KisQImagePyramid();

    static QSize imageSize(const QSize &baseSize,
                           qreal scale, qreal rotation,
                           qreal subPixelX, qreal subPixelY);

    QImage createImage(qreal scale, qreal rotation,
                       qreal subPixelX, qreal subPixelY);

private:
    int findNearestLevel(qreal scale, qreal *baseScale);
    static void calculateParams(qreal scale, qreal rotation,
                                qreal subPixelX, qreal subPixelY,
                                qreal baseScale, const QSize &baseSize,
                                QTransform *outputTransform, QSize *outputSize);

private:
    QSize m_baseSize;
    qreal m_baseScale;
    QVector<QImage> m_levels;
};

#endif /* __KIS_QIMAGE_PYRAMID_H */
