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

#ifndef __KIS_WARP_TRANSFORM_STRATEGY_H
#define __KIS_WARP_TRANSFORM_STRATEGY_H

#include <QObject>
#include <QScopedPointer>

#include "kis_simplified_action_policy_strategy.h"

class QPointF;
class QPainter;
class KisCoordinatesConverter;
class ToolTransformArgs;
class TransformTransactionProperties;
class QCursor;
class QImage;

enum TransformType {
    WARP_TRANSFORM,
    CAGE_TRANSFORM
};

class KisWarpTransformStrategy : public KisSimplifiedActionPolicyStrategy
{
    Q_OBJECT
public:
    KisWarpTransformStrategy(const KisCoordinatesConverter *converter,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisWarpTransformStrategy() override;

    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive) override;
    void setTransformType(TransformType type);

    void paint(QPainter &gc) override;
    QCursor getCurrentCursor() const override;

    void externalConfigChanged() override;

    using KisTransformStrategyBase::beginPrimaryAction;
    using KisTransformStrategyBase::continuePrimaryAction;
    using KisTransformStrategyBase::endPrimaryAction;

    bool beginPrimaryAction(const QPointF &pt) override;
    void continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive) override;
    bool endPrimaryAction() override;

    bool acceptsClicks() const override;

Q_SIGNALS:
    void requestCanvasUpdate();

protected:
    // default is true
    void setClipOriginalPointsPosition(bool value);

    // default is false
    void setCloseOnStartPointClick(bool value);

    void overrideDrawingItems(bool drawConnectionLines,
                              bool drawOrigPoints,
                              bool drawTransfPoints);

    virtual void drawConnectionLines(QPainter &gc,
                                     const QVector<QPointF> &origPoints,
                                     const QVector<QPointF> &transfPoints,
                                     bool isEditingPoints);

    virtual QImage calculateTransformedImage(ToolTransformArgs &currentArgs,
                                             const QImage &srcImage,
                                             const QVector<QPointF> &origPoints,
                                             const QVector<QPointF> &transfPoints,
                                             const QPointF &srcOffset,
                                             QPointF *dstOffset);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_WARP_TRANSFORM_STRATEGY_H */
