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
#include "kis_selection.h"
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
    QPushButton *buttonFillForegroundColor = nullptr;
    QPushButton *buttonCropToSelection = nullptr;
    KisSelectionManager *selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;
    QPoint dragRectPosition = QPoint(0, 0);
    bool dragging = false;
    QPoint dragStartOffset;
    int buttonCount = 7; // buttons + move handle
    int buttonSize = 25;
    int bufferSpace = 5;
    int actionBarWidth = buttonCount * buttonSize;
    bool selectionActive = false;
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
    QWidget *canvasWidget = dynamic_cast<QWidget*>(d->m_canvas->canvasWidget());
    canvasWidget->installEventFilter(this);

    KisSelectionSP selection = d->m_viewManager->selection();
    if (!d->selectionActive && selection) {
        d->selectionActive = true;
        QRectF selectionBounds = selection->selectedRect();
        int selectionBottom = selectionBounds.bottom();
        QPointF selectionCenter = selectionBounds.center();

        QPointF bottomCenter(selectionCenter.x(), selectionBottom);

        QPointF widgetBottomCenter = converter->imageToWidget(bottomCenter); // converts current selection's QPointF into canvasWidget's QPointF space
        widgetBottomCenter.setX(widgetBottomCenter.x() - (d->actionBarWidth / 2)); // centers toolbar midpoint with the selection center
        widgetBottomCenter.setY(widgetBottomCenter.y() + d->bufferSpace);

        d->dragRectPosition = widgetBottomCenter.toPoint();
    }

    d->dragRectPosition = updateCanvasBoundaries(d->dragRectPosition, canvasWidget);

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

    if (!d->buttonFillForegroundColor) {
        d->buttonFillForegroundColor = new QPushButton();
        d->buttonFillForegroundColor->setIcon(KisIconUtils::loadIcon("krita_tool_color_fill"));
        d->buttonFillForegroundColor->setFixedSize(25, 25);
        d->buttonFillForegroundColor->setToolTip("Fill Selection with Color");

        connect(d->buttonFillForegroundColor, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::fillForegroundColor);
    }

    if (!d->buttonCropToSelection) {
        d->buttonCropToSelection = new QPushButton();
        d->buttonCropToSelection->setIcon(KisIconUtils::loadIcon("tool_crop"));
        d->buttonCropToSelection->setFixedSize(25, 25);
        d->buttonCropToSelection->setToolTip("Crop to Selection");

        connect(d->buttonCropToSelection, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::imageResizeToSelection);
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

    if (canvasWidget && d->buttonFillForegroundColor && m_selectionActionBar) {
        d->buttonFillForegroundColor->setParent(canvasWidget);
        d->buttonFillForegroundColor->move(d->dragRectPosition.x() + 100, d->dragRectPosition.y());
        d->buttonFillForegroundColor->show();
    } else if (d->buttonFillForegroundColor) {
        d->buttonFillForegroundColor->hide();
    }

    if (canvasWidget && d->buttonCropToSelection && m_selectionActionBar) {
        d->buttonCropToSelection->setParent(canvasWidget);
        d->buttonCropToSelection->move(d->dragRectPosition.x() + 125, d->dragRectPosition.y());
        d->buttonCropToSelection->show();
    } else if (d->buttonCropToSelection) {
        d->buttonCropToSelection->hide();
    }

    if (!m_selectionActionBar) {
        d->selectionActive = false;
        return;
    }

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(d->dragRectPosition, QSize(25 * (d->buttonCount), 25)), 4, 4);
    gc.fillPath(bgPath, Qt::darkGray);

    QPen pen(QColor(60, 60, 60, 80));
    pen.setWidth(5);
    gc.setPen(pen);

    gc.drawPath(bgPath);

    QPainterPath dragRect;
    int width = 25;
    int height = 25;
    dragRect.addRect(QRectF(d->dragRectPosition.x() + (25 * d->buttonCount+1), d->dragRectPosition.y(), width, height));
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
    dragRectDots.translate(d->dragRectPosition.x() + 160, d->dragRectPosition.y() + 10);

    gc.fillPath(dragRectDots,dragDecorationDotsColor);
}

bool KisSelectionAssistantsDecoration::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *canvasWidget = dynamic_cast<QWidget*>(d->m_canvas->canvasWidget());
    if (obj != canvasWidget) return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QRect rect(d->dragRectPosition, QSize(25 * (d->buttonCount), 25));
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
        d->dragRectPosition = updateCanvasBoundaries(newPos, canvasWidget);
        canvasWidget->update();
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease && d->dragging) {
        d->dragging = false;
        return true;
    }

    return false;
}

QPoint KisSelectionAssistantsDecoration::updateCanvasBoundaries(QPoint position, QWidget *canvasWidget)
{
    QRect canvasBounds = canvasWidget->rect();
    int actionBarWidth = d->actionBarWidth;
    int actionBarHeight = d->buttonSize;
    int bufferSpace = d->bufferSpace;
    position.setX(qBound(canvasBounds.left() + bufferSpace, position.x(), canvasBounds.right() - actionBarWidth - bufferSpace));
    position.setY(qBound(canvasBounds.top() + bufferSpace, position.y(), canvasBounds.bottom() - actionBarHeight - bufferSpace));
    return position;
}
