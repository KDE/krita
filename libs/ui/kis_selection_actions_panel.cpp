/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_actions_panel.h"

#include "kis_canvas_widget_base.h"
#include "KisDocument.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_icon_utils.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
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

    KisSelectionManager *m_selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;

    bool m_dragging = false;
    bool m_visible = false;

    struct DragHandle {
        QPoint position = QPoint(0, 0);
        QPoint dragOrigin = QPoint(0, 0);
    };

    QScopedPointer<DragHandle> m_dragHandle;

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

KisSelectionActionsPanel::KisSelectionActionsPanel(KisViewManager *viewManager, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->m_viewManager = viewManager;
    d->m_selectionManager = viewManager->selectionManager();

    // Setup buttons...
    for (const ActionButtonData &buttonData : Private::buttonData()) {
        QPushButton *button = createButton(buttonData.iconName, buttonData.tooltip);
        connect(button, &QPushButton::clicked, d->m_selectionManager, buttonData.slot);
        d->m_buttons.append(button);
    }
}

KisSelectionActionsPanel::~KisSelectionActionsPanel()
{
    // buttons are children of the canvas, but we should still delete
    // them to make sure they are not accessed after the decoration dies
    qDeleteAll(d->m_buttons);
    d->m_buttons.clear();
}

void KisSelectionActionsPanel::draw(QPainter &painter)
{
    KisSelectionSP selection = d->m_viewManager->selection();

    if (!selection) {
        return;
    }

    drawActionBarBackground(painter);

    for (int i = 0; i < d->m_buttons.size(); i++) {
        QPushButton *button = d->m_buttons[i];
        int buttonPosition = i * BUTTON_SIZE;
        button->move(d->m_dragHandle->position.x() + buttonPosition, d->m_dragHandle->position.y());
        button->show();
    }
}

void KisSelectionActionsPanel::setVisible(bool p_visible)
{
    QWidget *canvasWidget = dynamic_cast<QWidget *>(d->m_viewManager->canvas());
    if (!canvasWidget) {
        return;
    }

    const bool VISIBILITY_CHANGED = d->m_visible != p_visible;
    if (!VISIBILITY_CHANGED) {
        return;
    }

    if (d->m_viewManager->selection() && p_visible) { // Now visible!
        canvasWidget->installEventFilter(this);

        d->m_dragHandle.reset(new Private::DragHandle());
        d->m_dragHandle->position = initialDragHandlePosition();

        for (QPushButton *button : d->m_buttons) {
            button->setParent(canvasWidget);
        }
    } else { // Now hidden!
        canvasWidget->removeEventFilter(this);

        for (QPushButton *button : d->m_buttons) {
            button->hide();
        }

        d->m_dragHandle.reset();
    }

    d->m_visible = p_visible;
}

bool KisSelectionActionsPanel::eventFilter(QObject *obj, QEvent *event)
{
    // Clicks...
    bool clickEvent = event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TabletPress || event->type() == QEvent::TouchBegin;

    if (clickEvent) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QRect dragHandleRect(d->m_dragHandle->position, QSize(25 * (d->m_buttonCount), 25));
        if (dragHandleRect.contains(mouseEvent->pos())) {
            d->m_dragging = true;
            d->m_dragHandle->dragOrigin = mouseEvent->pos() - d->m_dragHandle->position;

            return true;
        }
    }

    // Drags...
    bool dragEvent = d->m_dragging && (event->type() == QEvent::MouseMove || event->type() == QEvent::TouchUpdate);

    if (dragEvent) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint newPos = mouseEvent->pos() - d->m_dragHandle->dragOrigin;

        // bound actionBar to stay within canvas space
        QWidget *canvasWidget = dynamic_cast<QWidget *>(d->m_viewManager->canvas());

        if (obj != canvasWidget)
            return false;

        d->m_dragHandle->position = updateCanvasBoundaries(newPos, canvasWidget);
        canvasWidget->update();

        return true;
    }

    // Releases...
    bool releaseEvent = d->m_dragging && (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::TabletRelease || event->type() == QEvent::TouchEnd);

    if (releaseEvent) {
        d->m_dragging = false;

        return true;
    }

    return false;
}

QPoint KisSelectionActionsPanel::updateCanvasBoundaries(QPoint position, QWidget *canvasWidget) const
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

QPoint KisSelectionActionsPanel::initialDragHandlePosition() const
{
    KisSelectionSP selection = d->m_viewManager->selection();
    KisCanvasWidgetBase *canvas = dynamic_cast<KisCanvasWidgetBase*>(d->m_viewManager->canvas());
    KIS_ASSERT(selection);
    KIS_ASSERT(canvas);

    QRectF selectionBounds = selection->selectedRect();
    int selectionBottom = selectionBounds.bottom();
    QPointF selectionCenter = selectionBounds.center();
    QPointF bottomCenter(selectionCenter.x(), selectionBottom);

    QPointF widgetBottomCenter = canvas->coordinatesConverter()->imageToWidget(bottomCenter); // converts current selection's QPointF into canvasWidget's QPointF space

    widgetBottomCenter.setX(widgetBottomCenter.x() - (d->m_actionBarWidth / 2)); // centers toolbar midpoint with the selection center
    widgetBottomCenter.setY(widgetBottomCenter.y() + BUFFER_SPACE);

    return updateCanvasBoundaries(widgetBottomCenter.toPoint(), d->m_viewManager->canvas());
}

QPushButton *KisSelectionActionsPanel::createButton(const QString &iconName, const QString &tooltip)
{
    QPushButton *button = new QPushButton();
    button->setIcon(KisIconUtils::loadIcon(iconName));
    button->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    button->setToolTip(tooltip);
    return button;
}

void KisSelectionActionsPanel::drawActionBarBackground(QPainter &painter) const
{
    const int CORNER_RADIUS = 4;
    const int PEN_WIDTH = 5;
    const QColor BACKGROUND_COLOR = Qt::darkGray;
    const QColor OUTLINE_COLOR(60, 60, 60, 80);
    const QColor DOT_COLOR = Qt::lightGray;
    const int DOT_SIZE = 4;
    const int DOT_SPACING = 5;
    const QPoint DRAG_HANDLE_RECT_DOTS_OFFSET(10, 10);

    QRectF actionBarRect(d->m_dragHandle->position, QSize(d->m_actionBarWidth, BUTTON_SIZE));
    QPainterPath bgPath;
    bgPath.addRoundedRect(actionBarRect, CORNER_RADIUS, CORNER_RADIUS);
    painter.fillPath(bgPath, BACKGROUND_COLOR);

    QPen pen(OUTLINE_COLOR);
    pen.setWidth(PEN_WIDTH);
    painter.setPen(pen);
    painter.drawPath(bgPath);

    QRectF dragHandleRect(
        QPoint(d->m_dragHandle->position.x() + d->m_actionBarWidth - BUTTON_SIZE, d->m_dragHandle->position.y()),
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
