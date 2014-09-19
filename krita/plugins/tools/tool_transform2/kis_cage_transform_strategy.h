/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CAGE_TRANSFORM_STRATEGY_H
#define __KIS_CAGE_TRANSFORM_STRATEGY_H

#include <QObject>
#include <QScopedPointer>

#include "kis_warp_transform_strategy.h"

class QPointF;
class QPainter;
class KisCoordinatesConverter;
class ToolTransformArgs;
class QTransform;
class TransformTransactionProperties;
class QCursor;
class QImage;


class KisCageTransformStrategy : public KisWarpTransformStrategy
{
    Q_OBJECT
public:
    KisCageTransformStrategy(const KisCoordinatesConverter *converter,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisCageTransformStrategy();

protected:
    void drawConnectionLines(QPainter &gc,
                             const QVector<QPointF> &origPoints,
                             const QVector<QPointF> &transfPoints,
                             bool isEditingPoints);

    QImage calculateTransformedImage(ToolTransformArgs &currentArgs,
                                     const QImage &srcImage,
                                     const QVector<QPointF> &origPoints,
                                     const QVector<QPointF> &transfPoints,
                                     const QPointF &srcOffset,
                                     QPointF *dstOffset);

private:
    class Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CAGE_TRANSFORM_STRATEGY_H */
