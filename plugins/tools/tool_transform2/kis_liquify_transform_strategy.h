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

    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive);
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

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LIQUIFY_TRANSFORM_STRATEGY_H */
