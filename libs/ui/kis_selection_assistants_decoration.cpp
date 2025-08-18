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

struct ActionButtonData {
    QString iconName;
    QString tooltip;
    void (KisSelectionManager::*slot)();
};

struct KisSelectionAssistantsDecoration::Private {
    Private()
    {}
    KisCanvas2 * m_canvas = 0;
    KisSelectionManager *selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;
    QPoint dragRectPosition = QPoint(0, 0);
    bool dragging = false;
    QPoint dragStartOffset;
    bool selectionActive = false;
    int buttonSize = 25;
    int bufferSpace = 5;

    QVector<QPushButton*> buttons;
    static const QVector<ActionButtonData>& buttonData() {
        static const QVector<ActionButtonData> data = {
            { "select-all", "Select All", &KisSelectionManager::selectAll },
            { "select-clear", "Deselect", &KisSelectionManager::deselect },
            { "duplicatelayer", "Copy To New Layer", &KisSelectionManager::copySelectionToNewLayer },
            { "select-invert", "Invert Selection", &KisSelectionManager::invert },
            { "krita_tool_color_fill", "Fill Selection with Color", &KisSelectionManager::fillForegroundColor },
            { "tool_crop", "Crop to Selection", &KisSelectionManager::imageResizeToSelection }
        };
        return data;
    }
    int buttonCount = buttonData().size() + 1; // buttons + drag handle
    int actionBarWidth = buttonCount * buttonSize;
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

    setupButtons();

    for (int i = 0; i < d->buttons.size(); i++) {
        QPushButton *btn = d->buttons[i];
        if (canvasWidget && m_selectionActionBar) {
            int buttonPosition = i * d->buttonSize;
            btn->setParent(canvasWidget);
            btn->show();
            btn->move(d->dragRectPosition.x() + buttonPosition, d->dragRectPosition.y());
        } else {
            btn->hide();
        }
    }

    if (!m_selectionActionBar) {
        d->selectionActive = false;
        return;
    }

    drawActionBarBackground(gc);
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

QPushButton* KisSelectionAssistantsDecoration::createButton(const QString &iconName, const QString &tooltip)
{
    QPushButton *button = new QPushButton();
    button->setIcon(KisIconUtils::loadIcon(iconName));
    button->setFixedSize(d->buttonSize, d->buttonSize);
    button->setToolTip(tooltip);
    return button;
}

void KisSelectionAssistantsDecoration::setupButtons()
{
    if (!d->buttons.isEmpty()) return;

    for (const ActionButtonData &buttonData : Private::buttonData()) {
        QPushButton *button = createButton(buttonData.iconName, buttonData.tooltip);
        connect(button, &QPushButton::clicked, d->selectionManager, buttonData.slot);
        d->buttons.append(button);
    }
}

void KisSelectionAssistantsDecoration::drawActionBarBackground(QPainter& gc)
{
    int cornerRadius = 4;
    int penWidth = 5;
    QColor backgroundColor = Qt::darkGray;
    QColor outlineColor(60, 60, 60, 80);
    QColor dotColor = Qt::lightGray;
    int dotSize = 4;
    int dotSpacing = 5;
    QPoint dragHandleRectDotsOffset(10, 10);

    QRectF actionBarRect(d->dragRectPosition, QSize(d->actionBarWidth, d->buttonSize));
    QPainterPath bgPath;
    bgPath.addRoundedRect(actionBarRect, cornerRadius, cornerRadius);
    gc.fillPath(bgPath, backgroundColor);

    QPen pen(outlineColor);
    pen.setWidth(penWidth);
    gc.setPen(pen);
    gc.drawPath(bgPath);

    QRectF dragHandleRect(QPoint(d->dragRectPosition.x() + d->actionBarWidth - d->buttonSize, d->dragRectPosition.y()), QSize(d->buttonSize, d->buttonSize));
    QPainterPath dragHandlePath;
    dragHandlePath.addRect(dragHandleRect);
    gc.fillPath(dragHandlePath, backgroundColor);

    const std::list<std::pair<int, int>> offsets = {
        {0, 0},
        {dotSpacing, 0},
        {-dotSpacing, 0},
        {0, dotSpacing},
        {0, -dotSpacing},
        {dotSpacing, dotSpacing},
        {dotSpacing, -dotSpacing},
        {-dotSpacing, dotSpacing},
        {-dotSpacing, -dotSpacing}
    };

    QPainterPath dragHandleRectDots;
    for (const std::pair<int, int> &offset : offsets) {
        dragHandleRectDots.addEllipse(offset.first, offset.second, dotSize, dotSize);
    };

    dragHandleRectDots.translate(dragHandleRect.topLeft() + dragHandleRectDotsOffset);
    gc.fillPath(dragHandleRectDots, dotColor);
}
