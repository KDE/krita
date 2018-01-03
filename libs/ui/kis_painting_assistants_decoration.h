/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_PAINTING_ASSISTANTS_MANAGER_H_
#define _KIS_PAINTING_ASSISTANTS_MANAGER_H_

#include <QPointF>
#include <QColor>

#include "KoPointerEvent.h"
#include "KoSnapGuide.h"

#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisView;

class KisPaintingAssistantsDecoration;
typedef KisSharedPtr<KisPaintingAssistantsDecoration> KisPaintingAssistantsDecorationSP;

/// data for editor widget. This is shared between the decoration and assistant tool which needs hit box information
struct AssistantEditorData {
    const int moveIconSize = 32;
    const int deleteIconSize = 24;
    const int snapIconSize = 20;
    const QPointF moveIconPosition = QPointF(15, 15);
    const QPointF snapIconPosition = QPointF(54, 20);
    const QPointF deleteIconPosition = QPointF(83, 18);
};

/**
 * This class hold a list of painting assistants.
 * In the application flow, each canvas holds one of these classes to manage the assistants
 * There is an assistants manager, but that is higher up in the flow and makes sure each view gets one of these
 * Since this is off the canvas level, the decoration can be seen across all tools. The contents from here will be in
 * front of the kis_assistant_tool, which hold and displays the editor controls.
 *
 * Many of the events this receives such as adding and removing assistants comes from kis_assistant_tool
 */
class KRITAUI_EXPORT KisPaintingAssistantsDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisPaintingAssistantsDecoration(QPointer<KisView> parent);
    ~KisPaintingAssistantsDecoration() override;
    void addAssistant(KisPaintingAssistantSP assistant);
    void removeAssistant(KisPaintingAssistantSP assistant);
    void removeAll();
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    void endStroke();
    QList<KisPaintingAssistantHandleSP> handles();
    QList<KisPaintingAssistantSP> assistants();

    /// called when assistant editor is activated
    /// right now this happens when the assistants tool is selected
    void activateAssistantsEditor();


    /// called when assistant editor is deactivated
    /// right now this happens when the assistants tool is un-selected
    void deactivateAssistantsEditor();

    /// brings back if we are currently editing assistants or not
    /// useful for some assistants (like spline) that draw bezier curves
    bool isEditingAssistants();


    /// sets whether the main assistant is visible
    void setAssistantVisible(bool set);

    /// sets whether the preview is visible
    void setOutlineVisible(bool set);

    /// sets whether we snap to only one assistant
    void setOnlyOneAssistantSnap(bool assistant);

    /// returns assistant visibility
    bool assistantVisibility();

    /// returns preview visibility
    bool outlineVisibility();

    /// uncache all assistants
    void uncache();

    /// retrieves the assistants color specified in the tool options
    /// all assistants will share the same color
    QColor assistantsColor();

    int handleSize();
    void setHandleSize(int handleSize);


Q_SIGNALS:
    void assistantChanged();
public Q_SLOTS:

    /// toggles whether the assistant is active or not
    void toggleAssistantVisible();

    /// toggles whether there will be a preview of the assistant result when painting
    void toggleOutlineVisible();
    void setAssistantsColor(QColor color);
    QPointF snapToGuide(KoPointerEvent *e, const QPointF &offset, bool useModifiers);
    QPointF snapToGuide(const QPointF& pt, const QPointF &offset);

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas) override;
    void drawHandles(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);    
    void drawEditorWidget(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);

private:
    struct Private;
    Private* const d;
};

#endif
