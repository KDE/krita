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
    KisSelectionActionsPanel();
    ~KisSelectionActionsPanel();
    void drawDecoration(QPainter &gc,
                        const KisCoordinatesConverter *converter,
                        KisCanvas2 *canvas,
                        bool selectionActionBarEnabled);
    void setViewManager(KisViewManager *viewManager);
    bool eventFilter(QObject *obj, QEvent *event) override;
    QPoint updateCanvasBoundaries(QPoint position, QWidget *canvasWidget);
    QPushButton *createButton(const QString &iconName, const QString &tooltip);
    void setupButtons();
    void drawActionBarBackground(QPainter &gc);

private:
    struct Private;
    Private *const d;

Q_SIGNALS:
};

#endif
