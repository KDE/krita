/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class TransformTransactionProperties;
class QImage;


class KisCageTransformStrategy : public KisWarpTransformStrategy
{
    Q_OBJECT
public:
    KisCageTransformStrategy(const KisCoordinatesConverter *converter, KoSnapGuide *snapGuide,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisCageTransformStrategy() override;

protected:
    void drawConnectionLines(QPainter &gc,
                             const QVector<QPointF> &origPoints,
                             const QVector<QPointF> &transfPoints,
                             bool isEditingPoints) override;

    QImage calculateTransformedImage(ToolTransformArgs &currentArgs,
                                     const QImage &srcImage,
                                     const QVector<QPointF> &origPoints,
                                     const QVector<QPointF> &transfPoints,
                                     const QPointF &srcOffset,
                                     QPointF *dstOffset) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CAGE_TRANSFORM_STRATEGY_H */
