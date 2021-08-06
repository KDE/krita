/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    CAGE_TRANSFORM,
    MESH_TRANSFORM
};

class KisWarpTransformStrategy : public KisSimplifiedActionPolicyStrategy
{
    Q_OBJECT
public:
    KisWarpTransformStrategy(const KisCoordinatesConverter *converter,
                             KoSnapGuide *snapGuide,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisWarpTransformStrategy() override;

    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive) override;
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

private:
    Q_PRIVATE_SLOT(m_d, void recalculateTransformations());

Q_SIGNALS:
    void requestCanvasUpdate();
    void requestImageRecalculation();

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
