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
#include "kis_selection_actions_panel_button.h"
#include "kis_selection_actions_panel_handle.h"
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

static constexpr int BUTTON_SIZE = 30;
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

    KisSelectionActionsPanelHandle * m_handleWidget;
    QList<KisSelectionActionsPanelButton *> m_buttons;
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

KisSelectionActionsPanel::KisSelectionActionsPanel(KisViewManager *viewManager)
    : d(new Private)
{
    d->m_viewManager = viewManager;
    d->m_selectionManager = viewManager->selectionManager();

    // Setup buttons...
    for (const ActionButtonData &buttonData : Private::buttonData()) {
        KisSelectionActionsPanelButton *button = new KisSelectionActionsPanelButton(buttonData.iconName, buttonData.tooltip, BUTTON_SIZE, viewManager->canvas());
        connect(button, &QAbstractButton::clicked, d->m_selectionManager, buttonData.slot);
        d->m_buttons.append(button);
    }

    d->m_handleWidget = new KisSelectionActionsPanelHandle(BUTTON_SIZE, viewManager->canvas());
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
    Q_FOREACH(KisSelectionActionsPanelButton* button, d->m_buttons){
        button->draw(painter);
    }

    d->m_handleWidget->draw(painter);
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
        d->m_handleWidget->installEventFilter(this);

        if (!d->m_dragHandle) {
            d->m_dragHandle.reset(new Private::DragHandle());
            d->m_dragHandle->position = initialDragHandlePosition();
            movePanelWidgets();
        }
    } else { // Now hidden!
        d->m_handleWidget->removeEventFilter(this);

        for (KisSelectionActionsPanelButton *button : d->m_buttons) {
            button->hide();
        }
        d->m_handleWidget->hide();

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
            return handleMove(event, mouseEventPos(mouseEvent));
        }
        break;
    case QEvent::TabletMove:
        if (d->m_pressed) {
            const QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);
            return handleMove(event, tabletEventPos(tabletEvent));
        }
        break;
    case QEvent::TouchUpdate:
        if (d->m_pressed) {
            const QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QPoint pos;
            if (touchEventPos(touchEvent, pos)) {
                return handleMove(event, pos);
            }
        }
        break;

    case QEvent::MouseButtonRelease:
    case QEvent::TabletRelease:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        if (d->m_pressed) {
            d->m_handleWidget->set_held(false);
            d->m_pressed = false;
            event->accept();
            return true;
        }
        break;

    default:
        break;
    }
    return false;
}


void KisSelectionActionsPanel::canvasWidgetChanged(KisCanvasWidgetBase* canvas)
{

    Q_FOREACH(QWidget* btn, d->m_buttons)  {
        btn->setParent(canvas->widget());
        btn->show();
    }

    d->m_handleWidget->setParent(canvas->widget());
    d->m_handleWidget->show();
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

void KisSelectionActionsPanel::drawActionBarBackground(QPainter &painter) const
{
    const int cornerRadius = 4;
    QColor outlineColor = Qt::darkGray;
    const QColor bgColor = qApp->palette().window().color();
    QColor bgColorTrans = bgColor;
    bgColorTrans.setAlpha(80);
    const int outline_width = 4;

    //an outer 1px wide outline for contrast against the background
    QRectF contrastOutline(d->m_dragHandle->position - QPoint(outline_width + 1,outline_width + 1), QSize(d->m_actionBarWidth, BUTTON_SIZE) +QSize(outline_width + 1,outline_width + 1) * 2);
    QRectF midOutline(d->m_dragHandle->position - QPoint(outline_width,outline_width), QSize(d->m_actionBarWidth, BUTTON_SIZE) +QSize(outline_width,outline_width) * 2);
    //Add a bit of padding here for the icons
    QRectF centerBackground(d->m_dragHandle->position - QPoint(outline_width,outline_width) / 2, QSize(d->m_actionBarWidth - BUTTON_SIZE, BUTTON_SIZE) +QSize(outline_width,outline_width));
    QPainterPath bgPath;
    QPainterPath outlinePath;
    QPainterPath contrastOutlinePath;

    bgPath.addRoundedRect(centerBackground, cornerRadius, cornerRadius);
    outlinePath.addRoundedRect(midOutline, cornerRadius, cornerRadius);
    contrastOutlinePath.addRoundedRect(contrastOutline, cornerRadius, cornerRadius);

    painter.fillPath(contrastOutlinePath, bgColorTrans);
    painter.fillPath(outlinePath, outlineColor);
    painter.fillPath(bgPath, bgColor);
}

bool KisSelectionActionsPanel::handlePress(QEvent *event, const QPoint &pos, Qt::MouseButton button)
{
    if (d->m_pressed) {
        event->accept();
        return true;
    }

    if (button == Qt::LeftButton) {
        d->m_pressed = true;
        d->m_dragHandle->dragOrigin = pos - d->m_dragHandle->position;
        d->m_handleWidget->set_held(true);

        event->accept();
        return true;
    }

    return false;
}

bool KisSelectionActionsPanel::handleMove(QEvent *event, const QPoint &pos)
{
    QWidget *canvasWidget = d->m_viewManager->canvas();
    QPoint newPos = pos - d->m_dragHandle->dragOrigin;
    d->m_dragHandle->position = updateCanvasBoundaries(newPos, canvasWidget);
    movePanelWidgets();
    canvasWidget->update();
    event->accept();
    return true;
}

void KisSelectionActionsPanel::movePanelWidgets()
{
    d->m_handleWidget->move(d->m_dragHandle->position.x() + d->m_buttons.size() * BUTTON_SIZE, d->m_dragHandle->position.y());
    d->m_handleWidget->show();

    int i = 0;
    Q_FOREACH(KisSelectionActionsPanelButton *button, d->m_buttons) {
        int buttonPosition = i * BUTTON_SIZE;
        button->move(d->m_dragHandle->position.x() + buttonPosition, d->m_dragHandle->position.y());
        button->show();
        i++;
    }
}

QPoint KisSelectionActionsPanel::mouseEventPos(const QMouseEvent *mouseEvent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return transformHandleCoords(mouseEvent->position().toPoint());
#else
    return transformHandleCoords(mouseEvent->pos());
#endif
}

QPoint KisSelectionActionsPanel::tabletEventPos(const QTabletEvent *tabletEvent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return transformHandleCoords(tabletEvent->position().toPoint());
#else
    return transformHandleCoords(tabletEvent->pos());
#endif
}

bool KisSelectionActionsPanel::touchEventPos(const QTouchEvent *touchEvent, QPoint &outPos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (touchEvent->pointCount() < 1) {
        return false;
    } else {
        outPos = transformHandleCoords(touchEvent->points().first().position().toPoint());
        return true;
    }
#else
    const QList<QTouchEvent::TouchPoint> &touchPoints = touchEvent->touchPoints();
    if (touchPoints.isEmpty()) {
        return false;
    } else {
        outPos = transformHandleCoords(touchPoints.first().pos().toPoint());
        return true;
    }
#endif
}

constexpr QPoint KisSelectionActionsPanel::transformHandleCoords(QPoint pos) {
    return d->m_dragHandle->position + pos;
}
