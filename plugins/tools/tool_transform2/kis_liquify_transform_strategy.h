/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LIQUIFY_TRANSFORM_STRATEGY_H
#define __KIS_LIQUIFY_TRANSFORM_STRATEGY_H

#include <QObject>
#include <QScopedPointer>

#include "kis_transform_strategy_base.h"

class QPointF;
class QPainter;
class KisCoordinatesConverter;
class ToolTransformArgs;
class TransformTransactionProperties;
class QCursor;


class KisLiquifyTransformStrategy : public KisTransformStrategyBase
{
    Q_OBJECT
public:
    KisLiquifyTransformStrategy(const KisCoordinatesConverter *converter,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction, const KoCanvasResourceProvider *manager);
    ~KisLiquifyTransformStrategy() override;

    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive);
    void paint(QPainter &gc) override;
    QCursor getCurrentCursor() const override;
    QPainterPath getCursorOutline() const override;

    bool acceptsClicks() const override;

    void externalConfigChanged() override;

    bool beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    bool endPrimaryAction(KoPointerEvent *event) override;
    void hoverActionCommon(KoPointerEvent *event) override;

    void activateAlternateAction(KisTool::AlternateAction action) override;
    void deactivateAlternateAction(KisTool::AlternateAction action) override;

    bool beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;
    bool endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;

Q_SIGNALS:
    void requestCanvasUpdate();
    void requestUpdateOptionWidget();
    void requestCursorOutlineUpdate(const QPointF &imagePoint);
    void requestImageRecalculation();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LIQUIFY_TRANSFORM_STRATEGY_H */
