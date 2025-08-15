/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SELECTION_ASSISTANTS_DECORATION_H_
#define _KIS_SELECTION_ASSISTANTS_DECORATION_H_

#include <QPointF>
#include <QColor>
#include <QPushButton>
#include <QObject>
#include "kis_types.h"

#include "KoPointerEvent.h"
#include "KoSnapGuide.h"
#include "kis_icon_utils.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisCanvas2;
class KisCoordinatesConverter;
class KisViewManager;

class KisSelectionAssistantsDecoration;
typedef KisSharedPtr<KisSelectionAssistantsDecoration> KisSelectionAssistantsDecorationSP;

class KRITAUI_EXPORT KisSelectionAssistantsDecoration : public QObject
{
    Q_OBJECT
public:
    KisSelectionAssistantsDecoration();
    ~KisSelectionAssistantsDecoration();
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas, bool selectionActionBarVisible);
    void setViewManager(KisViewManager* viewManager);
    bool eventFilter(QObject *obj, QEvent *event) override;
    QPoint updateCanvasBoundaries(QPoint position, QWidget *canvasWidget);
    QPushButton* createButton(const QString &iconName, const QString &tooltip);
    void setupButtons();
    void drawActionBarBackground(QPainter& gc);

private:
    struct Private;
    Private* const d;

Q_SIGNALS:

};

#endif
