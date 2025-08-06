/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_assistants_decoration.h"

#include <limits>

#include <QList>
#include <QPointF>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kis_algebra_2d.h>
#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_icon_utils.h"
#include "KisViewManager.h"
#include <KoCompositeOpRegistry.h>
#include "kis_tool_proxy.h"
#include "kis_selection_manager.h"
#include <QTransform>
#include <QPushButton>

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

class KisSelectionManager;

struct KisSelectionAssistantsDecoration::Private {
    Private()
    {}
    KisCanvas2 * m_canvas = 0;
    QPushButton *buttonSelectAll = nullptr;
    QPushButton *buttonDeselect = nullptr;
    QPushButton *buttonCopyToNewLayer = nullptr;
    QPushButton *buttonInvert = nullptr;
    KisSelectionManager *selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;
    QPoint dragRectPosition = QPoint(200, 512);
    bool dragging = false;
    QPoint dragStartOffset;
};

KisSelectionAssistantsDecoration::KisSelectionAssistantsDecoration() :
    d(new Private)
{
}

KisSelectionAssistantsDecoration::~KisSelectionAssistantsDecoration()
{
    delete d;
}

void KisSelectionAssistantsDecoration::setViewManager(KisViewManager* viewManager) {
    d->m_viewManager = viewManager;
    d->selectionManager = viewManager->selectionManager();
}

void KisSelectionAssistantsDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas, bool m_selectionActionBar)
{
    Q_UNUSED(updateRect);
    d->m_canvas = canvas;
    QWidget *canvasWidget = dynamic_cast<QWidget*>(canvas->canvasWidget());
    canvasWidget->installEventFilter(this);

    if (!d->buttonSelectAll) {
        d->buttonSelectAll = new QPushButton();
        d->buttonSelectAll->setIcon(KisIconUtils::loadIcon("select-all"));
        d->buttonSelectAll->setFixedSize(25, 25);
        d->buttonSelectAll->setToolTip("Select All");

        connect(d->buttonSelectAll, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::selectAll);
        }

    if (!d->buttonDeselect) {
        d->buttonDeselect = new QPushButton();
        d->buttonDeselect->setIcon(KisIconUtils::loadIcon("select-clear"));
        d->buttonDeselect->setFixedSize(25, 25);
        d->buttonDeselect->setToolTip("Deselect");

        connect(d->buttonDeselect, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::deselect);
    }

    if (!d->buttonCopyToNewLayer) {
        d->buttonCopyToNewLayer = new QPushButton();
        d->buttonCopyToNewLayer->setIcon(KisIconUtils::loadIcon("duplicatelayer"));
        d->buttonCopyToNewLayer->setFixedSize(25, 25);
        d->buttonCopyToNewLayer->setToolTip("Copy To New Layer");

        connect(d->buttonCopyToNewLayer, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::copySelectionToNewLayer);
    }

    if (!d->buttonInvert) {
        d->buttonInvert = new QPushButton();
        d->buttonInvert->setFixedSize(25, 25);
        d->buttonInvert->setToolTip("Invert Selection");

        connect(d->buttonInvert, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::invert);
    }

    if (canvasWidget && d->buttonSelectAll && m_selectionActionBar) {
        d->buttonSelectAll->setParent(canvasWidget);
        d->buttonSelectAll->move(d->dragRectPosition.x(), d->dragRectPosition.y());
        d->buttonSelectAll->show();
    } else if (d->buttonSelectAll) {
        d->buttonSelectAll->hide();
    }

    if (canvasWidget && d->buttonDeselect && m_selectionActionBar) {
        d->buttonDeselect->setParent(canvasWidget);
        d->buttonDeselect->move(d->dragRectPosition.x() + 25, d->dragRectPosition.y());
        d->buttonDeselect->show();
    } else if (d->buttonDeselect) {
        d->buttonDeselect->hide();
    }

    if (canvasWidget && d->buttonCopyToNewLayer && m_selectionActionBar) {
        d->buttonCopyToNewLayer->setParent(canvasWidget);
        d->buttonCopyToNewLayer->move(d->dragRectPosition.x() + 50, d->dragRectPosition.y());
        d->buttonCopyToNewLayer->show();
    } else if (d->buttonCopyToNewLayer) {
        d->buttonCopyToNewLayer->hide();
    }

    if (canvasWidget && d->buttonInvert && m_selectionActionBar) {
        d->buttonInvert->setParent(canvasWidget);
        d->buttonInvert->move(d->dragRectPosition.x() + 75, d->dragRectPosition.y());
        d->buttonInvert->show();
    } else if (d->buttonInvert) {
        d->buttonInvert->hide();
    }

    if (!m_selectionActionBar) {
        return;
    }

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(d->dragRectPosition, QSize(125, 25)), 4, 4);
    gc.fillPath(bgPath, Qt::darkGray);

    QPen pen(QColor(60, 60, 60, 80));
    pen.setWidth(6);
    gc.setPen(pen);

    gc.drawPath(bgPath);

    QPainterPath dragRect;
    int width = 25;
    int height = 25;
    dragRect.addRect(QRectF(d->dragRectPosition.x() + 100, d->dragRectPosition.y(), width, height));
    gc.fillPath(bgPath.intersected(dragRect), Qt::darkGray);

    QPainterPath dragRectDots;
    QColor dragDecorationDotsColor(Qt::lightGray);
    int dotSize = 4;
    dragRectDots.addEllipse(0,0,dotSize,dotSize);
    dragRectDots.addEllipse(5,-5,dotSize,dotSize);
    dragRectDots.addEllipse(5,0,dotSize,dotSize);
    dragRectDots.addEllipse(5,5,dotSize,dotSize);
    dragRectDots.addEllipse(0,-5,dotSize,dotSize);
    dragRectDots.addEllipse(0,5,dotSize,dotSize);
    dragRectDots.addEllipse(-5,-5,dotSize,dotSize);
    dragRectDots.addEllipse(-5,0,dotSize,dotSize);
    dragRectDots.addEllipse(-5,5,dotSize,dotSize);
    dragRectDots.translate(d->dragRectPosition.x() + 110, d->dragRectPosition.y() + 10);

    gc.fillPath(dragRectDots,dragDecorationDotsColor);
}

bool KisSelectionAssistantsDecoration::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *canvasWidget = dynamic_cast<QWidget*>(d->m_canvas->canvasWidget());
    if (obj != canvasWidget) return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect rect(d->dragRectPosition, QSize(128, 67));
        if (rect.contains(mouseEvent->pos())) {
            d->dragging = true;
            d->dragStartOffset = mouseEvent->pos() - d->dragRectPosition;
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove && d->dragging) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint newPos = mouseEvent->pos() - d->dragStartOffset;

        // bound actionBar to stay within canvas space
        QRect canvasBounds = canvasWidget->rect();
        int actionBarWidth = 125;
        int actionBarHeight = 25;
        int bufferSpace = 5;
        newPos.setX(qBound(canvasBounds.left() + bufferSpace, newPos.x(), canvasBounds.right() - actionBarWidth - bufferSpace));
        newPos.setY(qBound(canvasBounds.top() + bufferSpace, newPos.y(), canvasBounds.bottom() - actionBarHeight - bufferSpace));

        d->dragRectPosition = newPos;
        canvasWidget->update();
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease && d->dragging) {
        d->dragging = false;
        return true;
    }

    return false;
}
