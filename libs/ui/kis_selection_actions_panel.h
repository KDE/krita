/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SELECTION_ACTIONS_PANEL_H_
#define _KIS_SELECTION_ACTIONS_PANEL_H_

#include "kis_types.h"
#include <QColor>
#include <QObject>
#include <QPointF>
#include <QPushButton>

#include "KoPointerEvent.h"
#include "KoSnapGuide.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_icon_utils.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisCanvas2;
class KisCoordinatesConverter;
class KisViewManager;

class KisSelectionActionsPanel;
typedef KisSharedPtr<KisSelectionActionsPanel> KisSelectionActionsPanelSP;

class KRITAUI_EXPORT KisSelectionActionsPanel : public QObject
{
    Q_OBJECT
public:
    KisSelectionActionsPanel() = delete;
    KisSelectionActionsPanel(KisViewManager *viewManager);
    ~KisSelectionActionsPanel();

    void draw(QPainter &painter);
    void setVisible(bool visible);
    void setEnabled(bool enabled);
    bool eventFilter(QObject *obj, QEvent *event) override;

    void canvasWidgetChanged(KisCanvasWidgetBase* canvas);
private:
    QPoint updateCanvasBoundaries(QPoint position, QWidget *canvasWidget) const;
    QPoint initialDragHandlePosition() const;
    void drawActionBarBackground(QPainter &gc) const;

    bool handlePress(QEvent *event, const QPoint &pos, Qt::MouseButton button = Qt::LeftButton);
    bool handleMove(QEvent *event, const QPoint &pos);
    bool handleRelease(QEvent *event, const QPoint &pos);

    ///Moves all the widgets that are a part of the panel
    void movePanelWidgets();

    QPoint mouseEventPos(const QMouseEvent *mouseEvent);
    QPoint tabletEventPos(const QTabletEvent *tabletEvent);
    // Touch events can have zero touch points when pressing an entire palm on
    // the screen on Android, so this returns false if this is a non-touch.
    bool touchEventPos(const QTouchEvent *touchEvent, QPoint &outPos);

    constexpr QPoint transformHandleCoords(QPoint pos);

    struct Private;
    QScopedPointer<Private> d;
};

#endif
