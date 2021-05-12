/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_painting_assistants_decoration.h"

#include <cfloat>

#include <QList>
#include <QPointF>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_icon_utils.h"
#include "KisViewManager.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

struct KisPaintingAssistantsDecoration::Private {
    Private()
        : assistantVisible(false)
        , outlineVisible(false)
        , snapOnlyOneAssistant(true)
        , firstAssistant(0)
        , aFirstStroke(false)
        , m_handleSize(14)
    {}

    bool assistantVisible;
    bool outlineVisible;
    bool snapOnlyOneAssistant;
    KisPaintingAssistantSP firstAssistant;
    KisPaintingAssistantSP selectedAssistant;
    bool aFirstStroke;
    bool m_isEditingAssistants = false;
    bool m_outlineVisible = false;
    int m_handleSize; // size of editor handles on assistants

    // move, visibility, delete icons for each assistant. These only display while the assistant tool is active
    // these icons will be covered by the kis_paintint_assistant_decoration with things like the perspective assistant

    AssistantEditorData toolData;

    QPixmap m_iconDelete = KisIconUtils::loadIcon("dialog-cancel").pixmap(toolData.deleteIconSize, toolData.deleteIconSize);
    QPixmap m_iconSnapOn = KisIconUtils::loadIcon("visible").pixmap(toolData.snapIconSize, toolData.snapIconSize);
    QPixmap m_iconSnapOff = KisIconUtils::loadIcon("novisible").pixmap(toolData.snapIconSize, toolData.snapIconSize);
    QPixmap m_iconMove = KisIconUtils::loadIcon("transform-move").pixmap(toolData.moveIconSize, toolData.moveIconSize);

    KisCanvas2 * m_canvas = 0;
};



KisPaintingAssistantsDecoration::KisPaintingAssistantsDecoration(QPointer<KisView> parent) :
    KisCanvasDecoration("paintingAssistantsDecoration", parent),
    d(new Private)
{
    setAssistantVisible(true);
    setOutlineVisible(true);
    setPriority(95);
    d->snapOnlyOneAssistant = true; //turn on by default.
}

KisPaintingAssistantsDecoration::~KisPaintingAssistantsDecoration()
{
    delete d;
}

void KisPaintingAssistantsDecoration::slotUpdateDecorationVisibility()
{
    const bool shouldBeVisible = !assistants().isEmpty();

    if (visible() != shouldBeVisible) {
        setVisible(shouldBeVisible);
    }
}

void KisPaintingAssistantsDecoration::addAssistant(KisPaintingAssistantSP assistant)
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    if (assistants.contains(assistant)) return;

    assistants.append(assistant);
    assistant->setAssistantGlobalColorCache(view()->document()->assistantsGlobalColor());

    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());
    emit assistantChanged();
}

void KisPaintingAssistantsDecoration::removeAssistant(KisPaintingAssistantSP assistant)
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    KIS_ASSERT_RECOVER_NOOP(assistants.contains(assistant));

    if (assistants.removeAll(assistant)) {
        view()->document()->setAssistants(assistants);
        setVisible(!assistants.isEmpty());
        emit assistantChanged();
    }
}

void KisPaintingAssistantsDecoration::removeAll()
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    assistants.clear();
    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());

    emit assistantChanged();
}

void KisPaintingAssistantsDecoration::setAssistants(const QList<KisPaintingAssistantSP> &assistants)
{
    Q_FOREACH (KisPaintingAssistantSP assistant, assistants) {
        assistant->setAssistantGlobalColorCache(view()->document()->assistantsGlobalColor());
    }
    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());

    emit assistantChanged();
}

void KisPaintingAssistantsDecoration::setAdjustedBrushPosition(const QPointF position)
{
    if (!assistants().empty()) {
        Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
            assistant->setAdjustedBrushPosition(position);
        }
    }
}


QPointF KisPaintingAssistantsDecoration::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{

    if (assistants().empty()) {
        // No assisants, so no adjustment
        return point;
    }

    // There is at least 1 assistant
    if (assistants().count() == 1) {
        // Things are easy when there is only one assistant
        if(assistants().first()->isSnappingActive() == true){
            bool snapSingle = d->snapOnlyOneAssistant && d->aFirstStroke;
            QPointF newpoint = assistants().first()->adjustPosition(point, strokeBegin, !snapSingle);
            // check for NaN
            if (newpoint.x() != newpoint.x()) return point;
            // Tell the assistant that its guidelines should
            // follow the adjusted bush position.
            assistants().first()->setFollowBrushPosition(true);

            // we need setting d->aFirstStroke even for one assistant, because now
            // single assistants can be made out of several inner ones (vide TwoPointPerspective).
            //this is here to be compatible with the movement in the perspective tool.
            qreal dx = point.x() - strokeBegin.x();
            qreal dy = point.y() - strokeBegin.y();
            if (dx * dx + dy * dy >= 4.0) {
                // allow some movement before snapping
                d->aFirstStroke=true;
            }
            return newpoint;
        } else {
            // One assisant, but it is not active, so no adjustment
            return point;
        }
    }

    // There is more than one assistant.
    // We have to chose one of these.
    // This is done by checking which assistant gives the an adjusted position
    // that is closest to the current position.

    QPointF best = point;
    double distance = DBL_MAX;
    if (!d->snapOnlyOneAssistant) {
        // In this mode the best assistant is constantly computed anew.
        Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
            if (assistant->isSnappingActive() == true){//this checks if the assistant in question has it's snapping boolean turned on//
                QPointF pt = assistant->adjustPosition(point, strokeBegin, true);
                // check for NaN
                if (pt.x() != pt.x()) continue;
                double dist = qAbs(pt.x() - point.x()) + qAbs(pt.y() - point.y());
                if (dist < distance) {
                    best = pt;
                    distance = dist;
                }
                assistant->setFollowBrushPosition(true);
            }
        }
    } else if (d->aFirstStroke==false) {
        // In this mode we compute the best assistant during the initial
        // movement and then keep using that assistant. (Called the first
        // assistant).
        Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
            if(assistant->isSnappingActive() == true){//this checks if the assistant in question has it's snapping boolean turned on//
                QPointF pt = assistant->adjustPosition(point, strokeBegin, true);
                if (pt.x() != pt.x()) continue;
                double dist = qAbs(pt.x() - point.x()) + qAbs(pt.y() - point.y());
                if (dist < distance) {
                    best = pt;
                    distance = dist;
                    d->firstAssistant = assistant;
                }
                assistant->setFollowBrushPosition(true);
            }
        }
    } else if (d->firstAssistant) {
        //make sure there's a first assistant to begin with.//
        QPointF newpoint = d->firstAssistant->adjustPosition(point, strokeBegin, false);
        // BUGFIX: 402535
        // assistants might return (NaN,NaN), must always check for that
        if (newpoint.x() == newpoint.x()) {
            // not a NaN
            best = newpoint;
        }
    } else {
        d->aFirstStroke=false;
    }

    //this is here to be compatible with the movement in the perspective tool.
    qreal dx = point.x() - strokeBegin.x();
    qreal dy = point.y() - strokeBegin.y();
    if (dx * dx + dy * dy >= 4.0) {
        // allow some movement before snapping
        d->aFirstStroke=true;
    }

    return best;
}

void KisPaintingAssistantsDecoration::endStroke()
{
    d->aFirstStroke = false;

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->endStroke();
    }
}

void KisPaintingAssistantsDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas)
{
    if(assistants().isEmpty()) {
        return; // no assistants to worry about, ok to exit
    }

    if (!canvas) {
        dbgFile<<"canvas does not exist in painting assistant decoration, you may have passed arguments incorrectly:"<<canvas;
    } else {
        d->m_canvas = canvas;
    }

    // the preview functionality for assistants. do not show while editing
    if (d->m_isEditingAssistants) {
        d->m_outlineVisible = false;
    }
    else {
        d->m_outlineVisible = outlineVisibility();
    }

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->drawAssistant(gc, updateRect, converter, true, canvas, assistantVisibility(), d->m_outlineVisible);

        if (isEditingAssistants()) {
            drawHandles(assistant, gc, converter);
        }
    }

    // draw editor controls on top of all assistant lines (why this code is last)
    if (isEditingAssistants()) {
        Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
            drawEditorWidget(assistant, gc, converter);
        }
     }
}

void KisPaintingAssistantsDecoration::drawHandles(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter)
{
        QTransform initialTransform = converter->documentToWidgetTransform();

        QColor colorToPaint = assistant->effectiveAssistantColor();

        Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->handles()) {


            QPointF transformedHandle = initialTransform.map(*handle);
            QRectF ellipse(transformedHandle -  QPointF(handleSize() * 0.5, handleSize() * 0.5), QSizeF(handleSize(), handleSize()));

            QPainterPath path;
            path.addEllipse(ellipse);

            gc.save();
            gc.setPen(Qt::NoPen);
            gc.setBrush(colorToPaint);
            gc.drawPath(path);
            gc.restore();
        }

         // some assistants have side handles like the vanishing point assistant
         Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->sideHandles()) {
             QPointF transformedHandle = initialTransform.map(*handle);
             QRectF ellipse(transformedHandle -  QPointF(handleSize() * 0.5, handleSize() * 0.5), QSizeF(handleSize(), handleSize()));

             QPainterPath path;
             path.addEllipse(ellipse);

             gc.save();
             gc.setPen(Qt::NoPen);
             gc.setBrush(colorToPaint);
             gc.drawPath(path);
             gc.restore();
         }
}

int KisPaintingAssistantsDecoration::handleSize()
{
    return  d->m_handleSize;
}

void KisPaintingAssistantsDecoration::setHandleSize(int handleSize)
{
    d->m_handleSize = handleSize;
}

QList<KisPaintingAssistantHandleSP> KisPaintingAssistantsDecoration::handles()
{
    QList<KisPaintingAssistantHandleSP> hs;
    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->handles()) {
            if (!hs.contains(handle)) {
                hs.push_back(handle);
            }
        }
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->sideHandles()) {
            if (!hs.contains(handle)) {
                hs.push_back(handle);
            }
        }
    }
    return hs;
}

QList<KisPaintingAssistantSP> KisPaintingAssistantsDecoration::assistants() const
{
    QList<KisPaintingAssistantSP> assistants;
    if (view()) {
        if (view()->document()) {
            assistants = view()->document()->assistants();
        }
    }
    return assistants;
}

bool KisPaintingAssistantsDecoration::hasPaintableAssistants() const
{
    return !assistants().isEmpty();
}

KisPaintingAssistantSP KisPaintingAssistantsDecoration::selectedAssistant()
{
    return d->selectedAssistant;
}

void KisPaintingAssistantsDecoration::setSelectedAssistant(KisPaintingAssistantSP assistant)
{
    d->selectedAssistant = assistant;
    emit selectedAssistantChanged();
}

void KisPaintingAssistantsDecoration::deselectAssistant()
{
    d->selectedAssistant.clear();
}


void KisPaintingAssistantsDecoration::setAssistantVisible(bool set)
{
    d->assistantVisible=set;
}

void KisPaintingAssistantsDecoration::setOutlineVisible(bool set)
{
    d->outlineVisible=set;
}

void KisPaintingAssistantsDecoration::setOnlyOneAssistantSnap(bool assistant)
{
    d->snapOnlyOneAssistant = assistant;
}

bool KisPaintingAssistantsDecoration::assistantVisibility()
{
    return d->assistantVisible;
}
bool KisPaintingAssistantsDecoration::outlineVisibility()
{
    return d->outlineVisible;
}
void KisPaintingAssistantsDecoration::uncache()
{
    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->uncache();
    }
}
void KisPaintingAssistantsDecoration::toggleAssistantVisible()
{
    setAssistantVisible(!assistantVisibility());
    uncache();
}

void KisPaintingAssistantsDecoration::toggleOutlineVisible()
{
    setOutlineVisible(!outlineVisibility());
}

QColor KisPaintingAssistantsDecoration::globalAssistantsColor()
{
    return view()->document()->assistantsGlobalColor();
}

void KisPaintingAssistantsDecoration::setGlobalAssistantsColor(QColor color)
{
    // view()->document() is referenced multiple times in this class
    // it is used to later store things in the KRA file when saving.
    view()->document()->setAssistantsGlobalColor(color);

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->setAssistantGlobalColorCache(color);
    }

    uncache();
}

void KisPaintingAssistantsDecoration::activateAssistantsEditor()
{
    setVisible(true); // this turns on the decorations in general. we leave it on at this point
    d->m_isEditingAssistants = true;
    uncache(); // updates visuals when editing
}

void KisPaintingAssistantsDecoration::deactivateAssistantsEditor()
{
    if (!d->m_canvas) {
        return;
    }

    d->m_isEditingAssistants = false; // some elements are hidden when we aren't editing
    uncache(); // updates visuals when not editing
}

bool KisPaintingAssistantsDecoration::isEditingAssistants()
{
    return d->m_isEditingAssistants;
}

QPointF KisPaintingAssistantsDecoration::snapToGuide(KoPointerEvent *e, const QPointF &offset, bool useModifiers)
{
    if (!d->m_canvas || !d->m_canvas->currentImage()) {
        return e->point;
    }


    KoSnapGuide *snapGuide = d->m_canvas->snapGuide();
    QPointF pos = snapGuide->snap(e->point, offset, useModifiers ? e->modifiers() : Qt::NoModifier);

    return pos;
}

QPointF KisPaintingAssistantsDecoration::snapToGuide(const QPointF& pt, const QPointF &offset)
{
    if (!d->m_canvas) {
         return pt;
    }


    KoSnapGuide *snapGuide = d->m_canvas->snapGuide();
    QPointF pos = snapGuide->snap(pt, offset, Qt::NoModifier);

    return pos;
}

/*
 * functions only used internally in this class
 * we potentially could make some of these inline to speed up performance
*/

void KisPaintingAssistantsDecoration::drawEditorWidget(KisPaintingAssistantSP assistant, QPainter& gc, const KisCoordinatesConverter *converter)
{
    if (!assistant->isAssistantComplete()) {
        return;
    }

    AssistantEditorData toolData; // shared const data for positioning and sizing

    QTransform initialTransform = converter->documentToWidgetTransform();

    QPointF actionsPosition = initialTransform.map(assistant->viewportConstrainedEditorPosition(converter, toolData.boundingSize));

    QPointF iconMovePosition(actionsPosition + toolData.moveIconPosition);
    QPointF iconSnapPosition(actionsPosition + toolData.snapIconPosition);
    QPointF iconDeletePosition(actionsPosition + toolData.deleteIconPosition);

    // Background container for helpers
    QBrush backgroundColor = d->m_canvas->viewManager()->mainWindow()->palette().window();
    QPointF actionsBGRectangle(actionsPosition + QPointF(10, 10));

    gc.setRenderHint(QPainter::Antialiasing);

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(actionsBGRectangle.x(), actionsBGRectangle.y(), toolData.boundingSize.width(), toolData.boundingSize.height()), 6, 6);
    QPen stroke(QColor(60, 60, 60, 80), 2);

    // if the assistant is selected, make outline stroke fatter and use theme's highlight color
    // for better visual feedback
    if (selectedAssistant()) { // there might not be a selected assistant, so do not seg fault
        if (assistant->getEditorPosition() == selectedAssistant()->getEditorPosition()) {
            stroke.setWidth(4);
            stroke.setColor(qApp->palette().color(QPalette::Highlight));
        }
    }

    // draw the final result
    gc.setPen(stroke);
    gc.fillPath(bgPath, backgroundColor);
    gc.drawPath(bgPath);


    // Move Assistant Tool helper
    gc.drawPixmap(iconMovePosition, d->m_iconMove);

    // active toggle
    if (assistant->isSnappingActive() == true) {
        gc.drawPixmap(iconSnapPosition, d->m_iconSnapOn);
    }
    else {
        gc.drawPixmap(iconSnapPosition, d->m_iconSnapOff);
    }

    gc.drawPixmap(iconDeletePosition, d->m_iconDelete);


}
