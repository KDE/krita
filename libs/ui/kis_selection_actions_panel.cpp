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
#include <QMouseEvent>
#include <QPointF>
#include <QPushButton>
#include <QTabletEvent>
#include <QTouchEvent>
#include <QTransform>
#include <kactioncollection.h>
#include <kis_algebra_2d.h>
#include <klocalizedstring.h>
#include <ktoggleaction.h>

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

static constexpr int BUTTON_SIZE = 25;
static constexpr int BUFFER_SPACE = 5;

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

    int m_pressedIndex = -1;
    bool m_pressed = false;
    bool m_visible = false;
    bool m_enabled = true;

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

KisSelectionActionsPanel::KisSelectionActionsPanel(KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    d->m_viewManager = viewManager;
    d->m_selectionManager = viewManager->selectionManager();

    QWidget *canvasWidget = dynamic_cast<QWidget *>(viewManager->canvas());

    // Setup buttons...
    for (const ActionButtonData &buttonData : Private::buttonData()) {
        QPushButton *button = createButton(buttonData.iconName, buttonData.tooltip);
        connect(button, &QPushButton::clicked, d->m_selectionManager, buttonData.slot);
        button->setParent(canvasWidget);
        d->m_buttons.append(button);
    }
}

KisSelectionActionsPanel::~KisSelectionActionsPanel()
{
}

void KisSelectionActionsPanel::draw(QPainter &painter)
{
    KisSelectionSP selection = d->m_viewManager->selection();

    if (!selection || !d->m_enabled || !d->m_visible) {
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

    p_visible &= d->m_enabled;

    const bool VISIBILITY_CHANGED = d->m_visible != p_visible;
    if (!VISIBILITY_CHANGED) {
        return;
    }

    if (d->m_viewManager->selection() && p_visible) { // Now visible!
        canvasWidget->installEventFilter(this);

        if (!d->m_dragHandle) {
            d->m_dragHandle.reset(new Private::DragHandle());
            d->m_dragHandle->position = initialDragHandlePosition();
        }
    } else { // Now hidden!
        canvasWidget->removeEventFilter(this);

        for (QPushButton *button : d->m_buttons) {
            button->hide();
        }

        d->m_pressed = false;
        d->m_dragHandle.reset();
    }

    d->m_visible = p_visible;
}

void KisSelectionActionsPanel::setEnabled(bool enabled)
{
    bool configurationChanged = enabled != d->m_enabled;
    d->m_enabled = enabled;
    if (configurationChanged) {
        // Reset visibility when configuration changes
        setVisible(d->m_visible);
    }
}

bool KisSelectionActionsPanel::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
        event->accept();
        return true;

    case QEvent::MouseButtonPress: {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        return handlePress(event, mouseEventPos(mouseEvent), mouseEvent->button());
    }
    case QEvent::TabletPress: {
        const QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);
        return handlePress(event, tabletEventPos(tabletEvent));
    }
    case QEvent::TouchBegin: {
        const QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QPoint pos;
        if (touchEventPos(touchEvent, pos)) {
            return handlePress(event, pos);
        }
        break;
    }

    case QEvent::MouseMove:
        if (d->m_pressed) {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            return handleMove(event, mouseEventPos(mouseEvent), obj);
        }
        break;
    case QEvent::TabletMove:
        if (d->m_pressed) {
            const QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);
            return handleMove(event, tabletEventPos(tabletEvent), obj);
        }
        break;
    case QEvent::TouchUpdate:
        if (d->m_pressed) {
            const QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QPoint pos;
            if (touchEventPos(touchEvent, pos)) {
                return handleMove(event, pos, obj);
            }
        }
        break;

    case QEvent::MouseButtonRelease:
    case QEvent::TabletRelease:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        if (d->m_pressed) {
            if (d->m_pressedIndex >= 0 && d->m_pressedIndex < d->m_buttons.size()) {
                // A button was pressed, trigger it on the next event loop.
                QTimer::singleShot(0, d->m_buttons[d->m_pressedIndex], &QPushButton::click);
            }
            d->m_pressed = false;
            d->m_pressedIndex = -1;
            event->accept();
            return true;
        }
        break;

    default:
        break;
    }
    return false;
}

QPoint KisSelectionActionsPanel::updateCanvasBoundaries(QPoint position, QWidget *canvasWidget) const
{
    QRect canvasBounds = canvasWidget->rect();

    const int ACTION_BAR_WIDTH = d->m_actionBarWidth;
    const int ACTION_BAR_HEIGHT = BUTTON_SIZE;

    int pos_x_min = canvasBounds.left() + BUFFER_SPACE;
    int pos_x_max = canvasBounds.right() - ACTION_BAR_WIDTH - BUFFER_SPACE;

    int pos_y_min = canvasBounds.top() + BUFFER_SPACE;
    int pos_y_max = canvasBounds.bottom() - ACTION_BAR_HEIGHT - BUFFER_SPACE;

    //Ensure that max is always bigger than min
    //If the window is small enough max could be smaller than min
    if (pos_x_max < pos_x_min) {
        pos_x_max = pos_x_min;
    }

    //It is pretty implausible for it to happen vertically but better safe than sorry
    if (pos_y_max < pos_y_min) {
        pos_y_max = pos_y_min;
    }

    position.setX(qBound(pos_x_min, position.x(), pos_x_max));
    position.setY(qBound(pos_y_min, position.y(), pos_y_max));

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

bool KisSelectionActionsPanel::handlePress(QEvent *event, const QPoint &pos, Qt::MouseButton button)
{
    if (d->m_pressed) {
        event->accept();
        return true;
    }

    if (button == Qt::LeftButton) {
        QRect targetRect(d->m_dragHandle->position, QSize(BUTTON_SIZE * d->m_buttonCount, BUTTON_SIZE));
        if (targetRect.contains(pos)) {
            d->m_pressed = true;

            d->m_pressedIndex = (pos.x() - targetRect.left()) / BUTTON_SIZE;
            if (d->m_pressedIndex < 0 || d->m_pressedIndex >= d->m_buttons.size()) {
                d->m_dragHandle->dragOrigin = pos - d->m_dragHandle->position;
            }

            event->accept();
            return true;
        }
    }

    return false;
}

bool KisSelectionActionsPanel::handleMove(QEvent *event, const QPoint &pos, QObject *obj)
{
    // Are we dragging the bar or was a button pressed?
    if (d->m_pressedIndex < 0 || d->m_pressedIndex >= d->m_buttons.size()) {
        // bound actionBar to stay within canvas space
        QWidget *canvasWidget = d->m_viewManager->canvas();

        // Explicitly casting just in case inheritance adjusts the pointer weirdly
        // (MSVC does spicy things like that sometimes to keep you on your toes.)
        if (obj == static_cast<QObject *>(canvasWidget)) {
            QPoint newPos = pos - d->m_dragHandle->dragOrigin;
            d->m_dragHandle->position = updateCanvasBoundaries(newPos, canvasWidget);
            canvasWidget->update();
            event->accept();
            return true;
        } else {
            return false;
        }
    } else {
        // Button was pressed, we're just waiting for a release.
        event->accept();
        return true;
    }
}

QPoint KisSelectionActionsPanel::mouseEventPos(const QMouseEvent *mouseEvent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return mouseEvent->position().toPoint();
#else
    return mouseEvent->pos();
#endif
}

QPoint KisSelectionActionsPanel::tabletEventPos(const QTabletEvent *tabletEvent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return tabletEvent->position().toPoint();
#else
    return tabletEvent->pos();
#endif
}

bool KisSelectionActionsPanel::touchEventPos(const QTouchEvent *touchEvent, QPoint &outPos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (touchEvent->pointCount() < 1) {
        return false;
    } else {
        outPos = touchEvent->points().first().position().toPoint();
        return true;
    }
#else
    const QList<QTouchEvent::TouchPoint> &touchPoints = touchEvent->touchPoints();
    if (touchPoints.isEmpty()) {
        return false;
    } else {
        outPos = touchPoints.first().pos().toPoint();
        return true;
    }
#endif
}
