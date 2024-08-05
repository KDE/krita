/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_painting_assistants_decoration.h"

#include <limits>

#include <QList>
#include <QPointF>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kis_algebra_2d.h>
#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_icon_utils.h"
#include "KisViewManager.h"
#include <KoCompositeOpRegistry.h>
#include "kis_tool_proxy.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

struct KisPaintingAssistantsDecoration::Private {
    Private()
        : assistantVisible(false)
        , outlineVisible(false)
        , snapOnlyOneAssistant(true)
        , snapEraser(false)
        , useCache(false)
        , firstAssistant(0)
        , m_handleSize(14)
    {}

    bool assistantVisible;
    bool outlineVisible;
    bool snapOnlyOneAssistant;
    bool snapEraser;
    bool useCache;
    KisPaintingAssistantSP firstAssistant;
    KisPaintingAssistantSP selectedAssistant;
    bool m_isEditingAssistants = false;
    int m_handleSize; // size of editor handles on assistants

    // move, visibility, delete icons for each assistant. These only display while the assistant tool is active
    // these icons will be covered by the kis_painting_assistant_decoration with things like the perspective assistant

    AssistantEditorData toolData;

    QPixmap m_iconDelete = KisIconUtils::loadIcon("deletelayer").pixmap(toolData.deleteIconSize, toolData.deleteIconSize);
    QPixmap m_iconSnapOn = KisIconUtils::loadIcon("visible").pixmap(toolData.snapIconSize, toolData.snapIconSize);
    QPixmap m_iconSnapOff = KisIconUtils::loadIcon("novisible").pixmap(toolData.snapIconSize, toolData.snapIconSize);
    QPixmap m_iconMove = KisIconUtils::loadIcon("transform-move").pixmap(toolData.moveIconSize, toolData.moveIconSize);
    QPixmap m_iconLockOn = KisIconUtils::loadIcon("layer-locked").pixmap(toolData.lockedIconSize, toolData.lockedIconSize);
    QPixmap m_iconLockOff = KisIconUtils::loadIcon("layer-unlocked").pixmap(toolData.lockedIconSize, toolData.lockedIconSize);
    QPixmap m_iconDragEditorWidget = KisIconUtils::loadIcon("gridbrush").pixmap(toolData.dragEditorWidgetIconSize, toolData.dragEditorWidgetIconSize);



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
    d->snapEraser = false;

    slotConfigChanged(); // load the initial config
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

void KisPaintingAssistantsDecoration::slotConfigChanged()
{
    KisConfig cfg(true);
    const KisConfig::AssistantsDrawMode drawMode = cfg.assistantsDrawMode();

    d->useCache =
        (drawMode == KisConfig::ASSISTANTS_DRAW_MODE_PIXMAP_CACHE) ||
        (drawMode == KisConfig::ASSISTANTS_DRAW_MODE_LARGE_PIXMAP_CACHE);
}

void KisPaintingAssistantsDecoration::addAssistant(KisPaintingAssistantSP assistant)
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    if (assistants.contains(assistant)) return;

    assistants.append(assistant);
    assistant->setAssistantGlobalColorCache(view()->document()->assistantsGlobalColor());

    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());
    Q_EMIT assistantChanged();
}

void KisPaintingAssistantsDecoration::removeAssistant(KisPaintingAssistantSP assistant)
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    KIS_ASSERT_RECOVER_NOOP(assistants.contains(assistant));

    if (assistants.removeAll(assistant)) {
        view()->document()->setAssistants(assistants);
        setVisible(!assistants.isEmpty());
        Q_EMIT assistantChanged();
    }
}

void KisPaintingAssistantsDecoration::removeAll()
{
    QList<KisPaintingAssistantSP> assistants = view()->document()->assistants();
    assistants.clear();
    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());

    Q_EMIT assistantChanged();
}

void KisPaintingAssistantsDecoration::setAssistants(const QList<KisPaintingAssistantSP> &assistants)
{
    Q_FOREACH (KisPaintingAssistantSP assistant, assistants) {
        assistant->setAssistantGlobalColorCache(view()->document()->assistantsGlobalColor());
    }
    view()->document()->setAssistants(assistants);
    setVisible(!assistants.isEmpty());

    Q_EMIT assistantChanged();
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
        // No assistants, so no adjustment
        return point;
    }

    if  (!d->snapEraser
        && (d->m_canvas->resourceManager()->resource(KoCanvasResource::CurrentEffectiveCompositeOp).toString() == COMPOSITE_ERASE)) {
        // No snapping if eraser snapping is disabled and brush is an eraser
        return point;
    }

    KisImageSP image = d->m_canvas->image();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(image, point);

    const KisCoordinatesConverter *converter = d->m_canvas->coordinatesConverter();
    const qreal moveThresholdPt = 4.0 / (converter->effectiveZoom() * qMax(image->xRes(), image->yRes()));

    QPointF best = point;
    qreal minSquareDistance = std::numeric_limits<qreal>::max();
    qreal secondSquareDistance = std::numeric_limits<qreal>::max();

    if (!d->snapOnlyOneAssistant || !d->firstAssistant) {
        // In these cases the best assistant meeds to be determined.
        int numSuitableAssistants = 0;
        KisPaintingAssistantSP bestAssistant;

        Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
            if (assistant->isSnappingActive() == true){ // the toggle button with eye icon to disable assistants
                QPointF newpoint = assistant->adjustPosition(point, strokeBegin, true, moveThresholdPt);
                // Assistants that can't or don't want to snap return NaN values (aside from possible numeric issues)
                // NOTE: It would be safer to also reject points too far outside canvas (or widget?) area,
                // because for example squashed concentric ellipses can currently shoot points far off.
                if (qIsNaN(newpoint.x()) ||  qIsNaN(newpoint.y())) {
                    continue;
                }
                ++numSuitableAssistants;
                qreal dist = kisSquareDistance(newpoint, point);
                if (dist < minSquareDistance) {
                    best = newpoint;
                    secondSquareDistance = minSquareDistance;
                    minSquareDistance = dist;
                    bestAssistant = assistant;
                } else if (dist < secondSquareDistance) {
                    secondSquareDistance = dist;
                }
                assistant->setFollowBrushPosition(true);
            }
        }

        // When there are multiple choices within moveThresholdPt, delay the decision until movement leaves
        // threshold to determine which curve follows cursor movement the closest. Assistants with multiple
        // snapping curves also need this movement to decide the best choice.
        // NOTE: It is currently not possible to tell painting tools that a decision is pending, or that
        // the active snapping curve changed, so certain artifact lines from snapping changes are unavoidable.

        if (numSuitableAssistants > 1 && KisAlgebra2D::norm(point - strokeBegin) <= moveThresholdPt
                && (sqrt(secondSquareDistance) < moveThresholdPt)) {
            return strokeBegin;
        } else if (numSuitableAssistants > 0 && d->snapOnlyOneAssistant) {
            // if only snapping to one assistant, register it
            d->firstAssistant = bestAssistant;
        }
    } else {
        // Make sure BUG:448187 doesn't crop up again
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->firstAssistant->isSnappingActive(), point);
        // there already is an assistant locked in, check if it can be used
        QPointF newpoint = d->firstAssistant->adjustPosition(point, strokeBegin, false, moveThresholdPt);
        // BUGFIX: 402535
        // assistants might return (NaN,NaN), must always check for that
        if (!(qIsNaN(newpoint.x()) || qIsNaN(newpoint.y()))) {
            best = newpoint;
        }
    }

    return best;
}

void KisPaintingAssistantsDecoration::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    if (assistants().empty()) {
        // No assistants, so no adjustment
        return;
    }

    // TODO: figure it out
    if  (!d->snapEraser
        && (d->m_canvas->resourceManager()->resource(KoCanvasResource::CurrentEffectiveCompositeOp).toString() == COMPOSITE_ERASE)) {
        // No snapping if eraser snapping is disabled and brush is an eraser
        return;
    }

    QPointF originalPoint = point;
    QPointF originalStrokeBegin = strokeBegin;

    qreal minDistance = 10000.0;
    bool minDistValid = false;
    QPointF finalPoint = originalPoint;
    QPointF finalStrokeBegin = originalStrokeBegin;
    int id = 0;
    KisPaintingAssistantSP bestAssistant;
    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        if(assistant->isSnappingActive() == true){//this checks if the assistant in question has it's snapping boolean turned on//
            //QPointF pt = assistant->adjustPosition(point, strokeBegin, true);
            QPointF p1 = originalPoint;
            QPointF p2 = originalStrokeBegin;
            assistant->adjustLine(p1, p2);
            if (p1.isNull() || p2.isNull()) {
                // possibly lines cannot snap to this assistant, or this line cannot, at least
                continue;
            }
            qreal distance = kisSquareDistance(p1, originalPoint) + kisSquareDistance(p2, originalStrokeBegin);
            if (distance < minDistance || !minDistValid) {
                finalPoint = p1;
                finalStrokeBegin = p2;
                minDistValid = true;
                bestAssistant = assistant;
            }
        }
        id ++;
    }
    if (bestAssistant) {
        bestAssistant->setFollowBrushPosition(true);
    }
    point = finalPoint;
    strokeBegin = finalStrokeBegin;
}

void KisPaintingAssistantsDecoration::endStroke()
{
    d->firstAssistant.clear();

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->endStroke();
    }
}

void KisPaintingAssistantsDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas)
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

    KoToolProxy *proxy = view()->canvasBase()->toolProxy();
    KIS_SAFE_ASSERT_RECOVER_RETURN(proxy);
    KisToolProxy *kritaProxy = dynamic_cast<KisToolProxy*>(proxy);
    KIS_SAFE_ASSERT_RECOVER_RETURN(kritaProxy);

    const bool outlineVisible =
        outlineVisibility() &&
        !d->m_isEditingAssistants &&
        kritaProxy->supportsPaintingAssistants();

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants()) {
        assistant->drawAssistant(gc, updateRect, converter, d->useCache, canvas, assistantVisibility(), outlineVisible);

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
    Q_EMIT selectedAssistantChanged();
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

void KisPaintingAssistantsDecoration::setEraserSnap(bool assistant)
{
    d->snapEraser = assistant;
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
    // "outline" means assistant preview (line that depends on the mouse cursor)
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
    QPointF iconLockedPosition(actionsPosition + toolData.lockedIconPosition);
    QPointF iconDeletePosition(actionsPosition + toolData.deleteIconPosition);
    QPointF iconDragEditorWidgetPosition(actionsPosition + toolData.dragEditorWidgetIconPosition);

    // Background container for helpers
    QBrush backgroundColor = d->m_canvas->viewManager()->mainWindowAsQWidget()->palette().window();
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

    if (assistant->isLocked()) {
        gc.drawPixmap(iconLockedPosition, d->m_iconLockOn);
    } else {
        qreal oldOpacity = gc.opacity();
        gc.setOpacity(0.35);
        gc.drawPixmap(iconLockedPosition, d->m_iconLockOff);
        gc.setOpacity(oldOpacity);
    }


    gc.drawPixmap(iconDeletePosition, d->m_iconDelete);
    gc.drawPixmap(iconDragEditorWidgetPosition, d->m_iconDragEditorWidget);


}
