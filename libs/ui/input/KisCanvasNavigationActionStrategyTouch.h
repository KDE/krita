/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCANVASNAVIGATIONACTIONSTRATEGYTOUCH_H
#define KISCANVASNAVIGATIONACTIONSTRATEGYTOUCH_H

#include "KisCanvasNavigationActionStrategy.h"

#include <KoViewTransformStillPoint.h>

#include <QPointF>

class QEvent;
class KisCanvas2;
class QTouchEvent;

class KisCanvasNavigationActionStrategyTouch : public KisCanvasNavigationActionStrategy
{
public:
    KisCanvasNavigationActionStrategyTouch(Flags flags, const QTouchEvent *startEvent, KisCanvas2 *canvas);

    bool supportsEvent(QEvent* event) const override;
    void inputEvent(QEvent* event) override;

private:
    KisCanvas2 *m_canvas;

    qreal m_nonRoundedZoom {0.0};
    KoViewTransformStillPoint m_actionStillPoint;

    SnappedRotationData m_rotationData;
    qreal m_lastDistance {0.0};
};



#endif // KISCANVASNAVIGATIONACTIONSTRATEGYTOUCH_H
