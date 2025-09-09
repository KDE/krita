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
#include "kis_icon_utils.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_painting_assistant.h"
#include <kritaui_export.h>

class KisView;

class KisPaintingAssistantsDecoration;
typedef KisSharedPtr<KisPaintingAssistantsDecoration> KisPaintingAssistantsDecorationSP;

// Data for editor widget. This is shared between the decoration and assistant tool which needs hit box information
struct AssistantEditorData {
    //button count to loop over
    unsigned int buttoncount = 6;
    //button icon size
    const int buttonSize = 24;
    //boolean values track which buttons are enabled within the editor widget
    bool moveButtonActivated = true;
    bool snapButtonActivated = true;
    bool lockButtonActivated = true;
    bool duplicateButtonActivated = true;
    bool deleteButtonActivated = true;
    bool widgetActivated = true;
    //padding for dynamic positioning between buttons
    const int buttonPadding = 7;
    //button positions
    QPointF moveIconPosition = QPointF(0, 0);
    QPointF snapIconPosition = QPointF(0, 0);
    QPointF lockedIconPosition = QPointF(0, 0);
    QPointF duplicateIconPosition = QPointF(0, 0);
    QPointF deleteIconPosition = QPointF(0, 0);
    QSize boundingSize = QSize(0, 0);
    //size of the side drag decoration
    const int dragDecorationWidth = 15;
    //QPixMaps representing icons for buttons
    const QPixmap m_iconMove = KisIconUtils::loadIcon("transform-move").pixmap(buttonSize+10, buttonSize+10);
    const QPixmap m_iconSnapOn = KisIconUtils::loadIcon("visible").pixmap(buttonSize, buttonSize);
    const QPixmap m_iconSnapOff = KisIconUtils::loadIcon("novisible").pixmap(buttonSize, buttonSize);
    const QPixmap m_iconLockOn = KisIconUtils::loadIcon("layer-locked").pixmap(buttonSize, buttonSize);
    const QPixmap m_iconLockOff = KisIconUtils::loadIcon("layer-unlocked").pixmap(buttonSize, buttonSize);
    const QPixmap m_iconDuplicate = KisIconUtils::loadIcon("duplicateitem").pixmap(buttonSize, buttonSize);
    const QPixmap m_iconDelete = KisIconUtils::loadIcon("deletelayer").pixmap(buttonSize, buttonSize);
    //how many buttons fit horizontally before extending to the next row
    const int horizontalButtonLimit = 3;
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
    void raiseAssistant(KisPaintingAssistantSP assistant);
    void removeAssistant(KisPaintingAssistantSP assistant);
    void removeAll();
    void setAssistants(const QList<KisPaintingAssistantSP> &assistants);
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    void adjustLine(QPointF &point, QPointF& strokeBegin);
    void setAdjustedBrushPosition(const QPointF position);
    void endStroke();
    QList<KisPaintingAssistantHandleSP> handles();
    QList<KisPaintingAssistantSP> assistants() const;
    //store the editor data to be used to control the render/interaction of the editor widget.
    struct AssistantEditorData globalEditorWidgetData;
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
    void slotConfigChanged();

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas) override;
    void drawHandles(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);
    void drawEditorWidget(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter);

private:
    struct Private;
    Private* const d;
};

#endif
