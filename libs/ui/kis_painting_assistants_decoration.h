/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    const QSize boundingSize = QSize(110, 40);
};

/**
 * KisPaintingAssistantsDecoration draws the assistants stored in the document on
 * the canvas.
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
    void setAssistants(const QList<KisPaintingAssistantSP> &assistants);
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    void setAdjustedBrushPosition(const QPointF position);
    void endStroke();
    QList<KisPaintingAssistantHandleSP> handles();
    QList<KisPaintingAssistantSP> assistants() const;

    bool hasPaintableAssistants() const;


    /// getter and setter functions for what assistant is currently selected
    /// this is used to control some tool options that are specific to a assistant
    KisPaintingAssistantSP selectedAssistant();
    void setSelectedAssistant(KisPaintingAssistantSP assistant);
    void deselectAssistant();


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

    /// sets whether eraser brushes snap
    void setEraserSnap(bool assistant);

    /// returns assistant visibility
    bool assistantVisibility();

    /// returns preview visibility
    bool outlineVisibility();

    /// uncache all assistants
    void uncache();

    int handleSize();
    void setHandleSize(int handleSize);

    QColor globalAssistantsColor();
    void setGlobalAssistantsColor(QColor color);

Q_SIGNALS:
    void assistantChanged();
    void selectedAssistantChanged();

public Q_SLOTS:

    /// toggles whether the assistant is active or not
    void toggleAssistantVisible();

    /// toggles whether there will be a preview of the assistant result when painting
    void toggleOutlineVisible();
    QPointF snapToGuide(KoPointerEvent *e, const QPointF &offset, bool useModifiers);
    QPointF snapToGuide(const QPointF& pt, const QPointF &offset);

    void slotUpdateDecorationVisibility();

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas) override;
    void drawHandles(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);
    void drawEditorWidget(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);

private:
    struct Private;
    Private* const d;
};

#endif
