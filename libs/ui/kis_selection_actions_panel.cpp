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

const int BUTTON_SIZE = 25;
const int BUFFER_SPACE = 5;

class KisSelectionManager;

struct ActionButtonData {
    QString iconName;
    QString tooltip;

    using TargetSlot = void (KisSelectionManager::*)();
    TargetSlot slot;
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

    QVector<QPushButton *> m_buttons;
    static const QVector<ActionButtonData> &buttonData()
    {
        static const QVector<ActionButtonData> data = {
            {"select-all", i18n("Select All"), &KisSelectionManager::selectAll},
            {"select-invert", i18n("Invert Selection"), &KisSelectionManager::invert},
            {"select-clear", i18n("Deselect"), &KisSelectionManager::deselect},
            {"krita_tool_color_fill", i18n("Fill Selection with Color"), &KisSelectionManager::fillForegroundColor},
            {"draw-eraser", i18n("Clear Selection"), &KisSelectionManager::clear},
            {"duplicatelayer", i18n("Copy To New Layer"), &KisSelectionManager::copySelectionToNewLayer},
            {"tool_crop", i18n("Crop to Selection"), &KisSelectionManager::imageResizeToSelection}};
        return data;
    }
    int m_buttonCount = buttonData().size() + 1; // buttons + drag handle

    int m_actionBarWidth = m_buttonCount * BUTTON_SIZE;
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

        widgetBottomCenter.setX(widgetBottomCenter.x() - (d->m_actionBarWidth / 2)); // centers toolbar midpoint with the selection center
        widgetBottomCenter.setY(widgetBottomCenter.y() + BUFFER_SPACE);

        d->m_dragRectPosition = widgetBottomCenter.toPoint();
    }

    d->m_dragRectPosition = updateCanvasBoundaries(d->m_dragRectPosition, canvasWidget);

    setupButtons();

    for (int i = 0; i < d->m_buttons.size(); i++) {
        QPushButton *btn = d->m_buttons[i];
        if (canvasWidget && selectionActionBarEnabled) {
            int buttonPosition = i * BUTTON_SIZE;
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

    const int ACTION_BAR_WIDTH = d->m_actionBarWidth;
    const int ACTION_BAR_HEIGHT = BUTTON_SIZE;

    position.setX(qBound(canvasBounds.left() + BUFFER_SPACE,
                         position.x(),
                         canvasBounds.right() - ACTION_BAR_WIDTH - BUFFER_SPACE));

    position.setY(qBound(canvasBounds.top() + BUFFER_SPACE,
                         position.y(),
                         canvasBounds.bottom() - ACTION_BAR_HEIGHT - BUFFER_SPACE));

    return position;
}

QPushButton *KisSelectionActionsPanel::createButton(const QString &iconName, const QString &tooltip)
{
    QPushButton *button = new QPushButton();
    button->setIcon(KisIconUtils::loadIcon(iconName));
    button->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
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

void KisSelectionActionsPanel::drawActionBarBackground(QPainter &painter)
{
    const int CORNER_RADIUS = 4;
    const int PEN_WIDTH = 5;
    const QColor BACKGROUND_COLOR = Qt::darkGray;
    const QColor OUTLINE_COLOR(60, 60, 60, 80);
    const QColor DOT_COLOR = Qt::lightGray;
    const int DOT_SIZE = 4;
    const int DOT_SPACING = 5;
    const QPoint DRAG_HANDLE_RECT_DOTS_OFFSET(10, 10);

    QRectF actionBarRect(d->m_dragRectPosition, QSize(d->m_actionBarWidth, BUTTON_SIZE));
    QPainterPath bgPath;
    bgPath.addRoundedRect(actionBarRect, CORNER_RADIUS, CORNER_RADIUS);
    painter.fillPath(bgPath, BACKGROUND_COLOR);

    QPen pen(OUTLINE_COLOR);
    pen.setWidth(PEN_WIDTH);
    painter.setPen(pen);
    painter.drawPath(bgPath);

    QRectF dragHandleRect(
        QPoint(d->m_dragRectPosition.x() + d->m_actionBarWidth - BUTTON_SIZE, d->m_dragRectPosition.y()),
        QSize(BUTTON_SIZE, BUTTON_SIZE));
    QPainterPath dragHandlePath;
    dragHandlePath.addRect(dragHandleRect);
    painter.fillPath(dragHandlePath, BACKGROUND_COLOR);

    const std::list<std::pair<int, int>> DOT_OFFSETS = {{0, 0},
                                                        {DOT_SPACING, 0},
                                                        {-DOT_SPACING, 0},
                                                        {0, DOT_SPACING},
                                                        {0, -DOT_SPACING},
                                                        {DOT_SPACING, DOT_SPACING},
                                                        {DOT_SPACING, -DOT_SPACING},
                                                        {-DOT_SPACING, DOT_SPACING},
                                                        {-DOT_SPACING, -DOT_SPACING}};

    QPainterPath dragHandleRectDots;
    for (const std::pair<int, int> &offset : DOT_OFFSETS) {
        dragHandleRectDots.addEllipse(offset.first, offset.second, DOT_SIZE, DOT_SIZE);
    };

    dragHandleRectDots.translate(dragHandleRect.topLeft() + DRAG_HANDLE_RECT_DOTS_OFFSET);
    painter.fillPath(dragHandleRectDots, DOT_COLOR);
}
