/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCANVASNAVIGATIONACTIONSTRATEGYNATIVEGESTURE_H
#define KISCANVASNAVIGATIONACTIONSTRATEGYNATIVEGESTURE_H

#include <QFlags>
#include <QPointF>
#include <KoViewTransformStillPoint.h>
#include "KisCanvasNavigationActionStrategy.h"

class QEvent;
class KisCanvas2;

class KisCanvasNavigationActionStrategyNativeGesture : public KisCanvasNavigationActionStrategy
{
public:
    enum Flag {
        None = 0x0,
        PanEnabled = 0x1,
        ZoomEnabled = 0x2,
        RotationEnabled = 0x4,
        ZoomDescrete = 0x8,
        RotationDescrete = 0x10,
    };
    Q_DECLARE_FLAGS(Flags, Flag)
public:
    KisCanvasNavigationActionStrategyNativeGesture(Flags flags, const QPointF &startViewPos, KisCanvas2 *canvas);

    bool supportsEvent(QEvent* event) const override;
    void inputEvent(QEvent* event) override;

private:
    KisCanvas2 *m_canvas;
    Flags m_flags;

    QPointF m_accumulatedPan;
    qreal m_nonRoundedZoom {0.0};
    qreal m_nonRoundedRelativeRotation {0.0};
    KoViewTransformStillPoint m_actionStillPoint;

    SnappedRotationData m_rotationData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisCanvasNavigationActionStrategyNativeGesture::Flags)

#endif // KISCANVASNAVIGATIONACTIONSTRATEGYNATIVEGESTURE_H
