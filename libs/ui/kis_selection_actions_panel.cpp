/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_actions_panel.h"

#include <limits>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_debug.h"
#include "kis_icon_utils.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_tool_proxy.h"
#include <KoCompositeOpRegistry.h>
#include <QList>
#include <QPointF>
#include <QPushButton>
#include <QTransform>
#include <kactioncollection.h>
#include <kis_algebra_2d.h>
#include <klocalizedstring.h>
#include <ktoggleaction.h>

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

class KisSelectionManager;

struct ActionButtonData {
    QString iconName;
    QString tooltip;
    void (KisSelectionManager::*slot)();
};

struct KisSelectionActionsPanel::Private {
    Private()
    {
    }
    KisCanvas2 *m_canvas = 0;
    KisSelectionManager *m_selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;
    QPoint m_dragRectPosition = QPoint(0, 0);
    bool m_dragging = false;
    QPoint m_dragStartOffset;
    bool m_selectionActive = false;
    int m_buttonSize = 25;
    int m_bufferSpace = 5;

    QVector<QPushButton *> m_buttons;
    static const QVector<ActionButtonData> &buttonData()
    {
        static const QVector<ActionButtonData> data = {
            {"select-all", i18nc("tooltip", "Select All"), &KisSelectionManager::selectAll},
            {"select-clear", i18nc("tooltip", "Deselect"), &KisSelectionManager::deselect},
            {"duplicatelayer", i18nc("tooltip", "Copy To New Layer"), &KisSelectionManager::copySelectionToNewLayer},
            {"select-invert", i18nc("tooltip", "Invert Selection"), &KisSelectionManager::invert},
            {"krita_tool_color_fill",
             i18nc("tooltip", "Fill Selection with Color"),
             &KisSelectionManager::fillForegroundColor},
            {"tool_crop", i18nc("tooltip", "Crop to Selection"), &KisSelectionManager::imageResizeToSelection}};
        return data;
    }
    int m_buttonCount = buttonData().size() + 1; // buttons + drag handle
    int m_actionBarWidth = m_buttonCount * m_buttonSize;
};

KisSelectionActionsPanel::KisSelectionActionsPanel()
    : d(new Private)
{
}

KisSelectionActionsPanel::~KisSelectionActionsPanel()
{
    delete d;
}

void KisSelectionActionsPanel::setViewManager(KisViewManager *viewManager)
{
    d->m_viewManager = viewManager;
    d->m_selectionManager = viewManager->selectionManager();
}

void KisSelectionActionsPanel::drawDecoration(QPainter &gc,
                                                      const KisCoordinatesConverter *converter,
                                                      KisCanvas2 *canvas,
                                                      bool selectionActionBarEnabled)
{
    d->m_canvas = canvas;
    QWidget *canvasWidget = dynamic_cast<QWidget *>(d->m_canvas->canvasWidget());
    canvasWidget->installEventFilter(this);

    KisSelectionSP selection = d->m_viewManager->selection();
    if (!d->m_selectionActive && selection) {
        d->m_selectionActive = true;
        QRectF selectionBounds = selection->selectedRect();
        int selectionBottom = selectionBounds.bottom();
        QPointF selectionCenter = selectionBounds.center();

        QPointF bottomCenter(selectionCenter.x(), selectionBottom);

        QPointF widgetBottomCenter = converter->imageToWidget(
            bottomCenter); // converts current selection's QPointF into canvasWidget's QPointF space
        widgetBottomCenter.setX(widgetBottomCenter.x()
                                - (d->m_actionBarWidth / 2)); // centers toolbar midpoint with the selection center
        widgetBottomCenter.setY(widgetBottomCenter.y() + d->m_bufferSpace);

        d->m_dragRectPosition = widgetBottomCenter.toPoint();
    }

    d->m_dragRectPosition = updateCanvasBoundaries(d->m_dragRectPosition, canvasWidget);

    setupButtons();

    for (int i = 0; i < d->m_buttons.size(); i++) {
        QPushButton *btn = d->m_buttons[i];
        if (canvasWidget && selectionActionBarEnabled) {
            int buttonPosition = i * d->m_buttonSize;
            btn->setParent(canvasWidget);
            btn->show();
            btn->move(d->m_dragRectPosition.x() + buttonPosition, d->m_dragRectPosition.y());
        } else {
            btn->hide();
        }
    }

    if (!selectionActionBarEnabled) {
        d->m_selectionActive = false;
        return;
    }

    drawActionBarBackground(gc);
}

bool KisSelectionActionsPanel::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *canvasWidget = dynamic_cast<QWidget *>(d->m_canvas->canvasWidget());
    if (obj != canvasWidget)
        return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QRect rect(d->m_dragRectPosition, QSize(25 * (d->m_buttonCount), 25));
        if (rect.contains(mouseEvent->pos())) {
            d->m_dragging = true;
            d->m_dragStartOffset = mouseEvent->pos() - d->m_dragRectPosition;
            return true;
        }
    }

    if (event->type() == QEvent::MouseMove && d->m_dragging) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint newPos = mouseEvent->pos() - d->m_dragStartOffset;

        // bound actionBar to stay within canvas space
        d->m_dragRectPosition = updateCanvasBoundaries(newPos, canvasWidget);
        canvasWidget->update();
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease && d->m_dragging) {
        d->m_dragging = false;
        return true;
    }

    return false;
}

QPoint KisSelectionActionsPanel::updateCanvasBoundaries(QPoint position, QWidget *canvasWidget)
{
    QRect canvasBounds = canvasWidget->rect();
    int m_actionBarWidth = d->m_actionBarWidth;
    int actionBarHeight = d->m_buttonSize;
    int m_bufferSpace = d->m_bufferSpace;
    position.setX(qBound(canvasBounds.left() + m_bufferSpace,
                         position.x(),
                         canvasBounds.right() - m_actionBarWidth - m_bufferSpace));
    position.setY(qBound(canvasBounds.top() + m_bufferSpace,
                         position.y(),
                         canvasBounds.bottom() - actionBarHeight - m_bufferSpace));
    return position;
}

QPushButton *KisSelectionActionsPanel::createButton(const QString &iconName, const QString &tooltip)
{
    QPushButton *button = new QPushButton();
    button->setIcon(KisIconUtils::loadIcon(iconName));
    button->setFixedSize(d->m_buttonSize, d->m_buttonSize);
    button->setToolTip(tooltip);
    return button;
}

void KisSelectionActionsPanel::setupButtons()
{
    if (!d->m_buttons.isEmpty())
        return;

    for (const ActionButtonData &buttonData : Private::buttonData()) {
        QPushButton *button = createButton(buttonData.iconName, buttonData.tooltip);
        connect(button, &QPushButton::clicked, d->m_selectionManager, buttonData.slot);
        d->m_buttons.append(button);
    }
}

void KisSelectionActionsPanel::drawActionBarBackground(QPainter &gc)
{
    int cornerRadius = 4;
    int penWidth = 5;
    QColor backgroundColor = Qt::darkGray;
    QColor outlineColor(60, 60, 60, 80);
    QColor dotColor = Qt::lightGray;
    int dotSize = 4;
    int dotSpacing = 5;
    QPoint dragHandleRectDotsOffset(10, 10);

    QRectF actionBarRect(d->m_dragRectPosition, QSize(d->m_actionBarWidth, d->m_buttonSize));
    QPainterPath bgPath;
    bgPath.addRoundedRect(actionBarRect, cornerRadius, cornerRadius);
    gc.fillPath(bgPath, backgroundColor);

    QPen pen(outlineColor);
    pen.setWidth(penWidth);
    gc.setPen(pen);
    gc.drawPath(bgPath);

    QRectF dragHandleRect(
        QPoint(d->m_dragRectPosition.x() + d->m_actionBarWidth - d->m_buttonSize, d->m_dragRectPosition.y()),
        QSize(d->m_buttonSize, d->m_buttonSize));
    QPainterPath dragHandlePath;
    dragHandlePath.addRect(dragHandleRect);
    gc.fillPath(dragHandlePath, backgroundColor);

    const std::list<std::pair<int, int>> offsets = {{0, 0},
                                                    {dotSpacing, 0},
                                                    {-dotSpacing, 0},
                                                    {0, dotSpacing},
                                                    {0, -dotSpacing},
                                                    {dotSpacing, dotSpacing},
                                                    {dotSpacing, -dotSpacing},
                                                    {-dotSpacing, dotSpacing},
                                                    {-dotSpacing, -dotSpacing}};

    QPainterPath dragHandleRectDots;
    for (const std::pair<int, int> &offset : offsets) {
        dragHandleRectDots.addEllipse(offset.first, offset.second, dotSize, dotSize);
    };

    dragHandleRectDots.translate(dragHandleRect.topLeft() + dragHandleRectDotsOffset);
    gc.fillPath(dragHandleRectDots, dotColor);
}
