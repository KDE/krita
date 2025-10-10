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
    KisSelectionActionsPanel(KisViewManager *viewManager, QObject *parent);
    ~KisSelectionActionsPanel();

    void draw(QPainter &painter,
            const KisCoordinatesConverter *coordinatesConverter);
    void setVisible(bool visible);
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupButtons();
    QPushButton *createButton(const QString &iconName, const QString &tooltip);
    QPoint updateCanvasBoundaries(QPoint position, QWidget *canvasWidget) const;
    QPoint initialDragHandlePosition() const;
    void drawActionBarBackground(QPainter &gc) const;

    struct Private;
    QScopedPointer<Private> d;

Q_SIGNALS:
};

#endif
