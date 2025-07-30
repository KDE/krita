/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoZoomStrategy.h"
#include "KoShapeRubberSelectStrategy_p.h"
#include "KoZoomTool.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"
#include "KoViewTransformStillPoint.h"

#include <FlakeDebug.h>

#include <QTransform>

KoZoomStrategy::KoZoomStrategy(KoZoomTool *tool, KoCanvasController *controller, const QPointF &clicked)
        : KoShapeRubberSelectStrategy(tool, clicked, false),
        m_controller(controller),
        m_forceZoomOut(false)
{
}

void KoZoomStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_D(KoShapeRubberSelectStrategy);

    const QTransform documentToWidget =
            m_controller->canvas()->viewConverter()->documentToView() *
            m_controller->canvas()->viewConverter()->viewToWidget();

    const QRect pixelRect = documentToWidget.mapRect(d->selectedRect()).toRect();

    bool m_zoomOut = m_forceZoomOut;
    if (modifiers & Qt::ControlModifier) {
        m_zoomOut = !m_zoomOut;
    }

    auto makeStillPoint = [&] () -> KoViewTransformStillPoint {
        const QPointF center = pixelRect.center();
        return { m_controller->canvas()->viewConverter()->viewToDocument(center), center };
    };

    if (m_zoomOut) {
        m_controller->zoomOut(makeStillPoint());
    } else if (pixelRect.width() > 5 && pixelRect.height() > 5) {
        m_controller->zoomTo(pixelRect);
    } else {
        m_controller->zoomIn(makeStillPoint());
    }
}

void KoZoomStrategy::cancelInteraction()
{
    Q_D(KoShapeRubberSelectStrategy);
    d->tool->canvas()->updateCanvas(d->selectedRect().normalized() | d->tool->decorationsRect());
}

KoShapeRubberSelectStrategy::SelectionMode KoZoomStrategy::currentMode() const
{
    return CoveringSelection;
}

void KoZoomStrategy::forceZoomOut()
{
    m_forceZoomOut = true;
}

void KoZoomStrategy::forceZoomIn()
{
    m_forceZoomOut = false;
}
