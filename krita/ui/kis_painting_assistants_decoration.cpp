/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_painting_assistants_decoration.h"

#include <cfloat>

#include <QList>
#include <QPointF>
#include <klocale.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include "kis_debug.h"

#include "kis_painting_assistant.h"
#include <QPainter>

struct KisPaintingAssistantsDecoration::Private {
    QList<KisPaintingAssistant*> assistants;
    KToggleAction* toggleAssistant;
    KToggleAction* togglePreview;
    bool assistantVisible;
    bool outlineVisible;
};

KisPaintingAssistantsDecoration::KisPaintingAssistantsDecoration(KisView2* parent) :
        KisCanvasDecoration("paintingAssistantsDecoration", parent),
        d(new Private)
{
}

KisPaintingAssistantsDecoration::~KisPaintingAssistantsDecoration()
{
    qDeleteAll(d->assistants.begin(), d->assistants.end());
    delete d;
}

void KisPaintingAssistantsDecoration::addAssistant(KisPaintingAssistant* assistant)
{
    if (d->assistants.contains(assistant)) return;
    d->assistants.push_back(assistant);
    updateAction();
}

void KisPaintingAssistantsDecoration::removeAssistant(KisPaintingAssistant* assistant)
{
    delete assistant;
    d->assistants.removeAll(assistant);
    updateAction();
}

void KisPaintingAssistantsDecoration::removeAll()
{
    foreach (KisPaintingAssistant* assistant, d->assistants) {
        delete assistant;
    }
    d->assistants.clear();
    updateAction();
}

QPointF KisPaintingAssistantsDecoration::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{
    if (d->assistants.empty()) return point;
    if (d->assistants.count() == 1) {
        if(d->assistants.first()->snapping()==true){
        QPointF newpoint = d->assistants.first()->adjustPosition(point, strokeBegin);
        // check for NaN
        if (newpoint.x() != newpoint.x()) return point;
        return newpoint;
        }
    }
    QPointF best = point;
    double distance = DBL_MAX;
    //the following tries to find the closest point to stroke-begin. It checks all assistants for the closest point//
    foreach(KisPaintingAssistant* assistant, d->assistants) {
        if(assistant->snapping()==true){//this checks if the assistant in question has it's snapping boolean turned on//
            QPointF pt = assistant->adjustPosition(point, strokeBegin);
            if (pt.x() != pt.x()) continue;
            double d = qAbs(pt.x() - point.x()) + qAbs(pt.y() - point.y());
            if (d < distance) {
                best = pt;
                distance = d;
            }
        }
    }
    return best;
}

void KisPaintingAssistantsDecoration::endStroke()
{
    foreach(KisPaintingAssistant* assistant, d->assistants) {
        assistant->endStroke();
    }
}

void KisPaintingAssistantsDecoration::setup(KActionCollection * collection)
{

    d->toggleAssistant = new KToggleAction(i18n("Show Painting Assistants"), this);
    d->togglePreview = new KToggleAction(i18n("Show Assistant Previews"), this);
    collection->addAction("view_toggle_painting_assistants", d->toggleAssistant);
    collection->addAction("view_toggle_assistant_previews", d->togglePreview);
    if (QMetaObject::invokeMethod(this, "toggleAssistantVisible")){
        connect(d->toggleAssistant, SIGNAL(triggered()), this, SLOT(toggleAssistantVisible()));
        connect(d->togglePreview, SIGNAL(triggered()), this, SLOT(toggleOutlineVisible()));
    }
    else {
        connect(d->toggleAssistant, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
    }
    setAssistantVisible(true);
    setOutlineVisible(true);
    updateAction();
}

void KisPaintingAssistantsDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas)
{   
    if (!canvas) {
        dbgFile<<"canvas does not exist in painting assistant decoration, you may have passed arguments incorrectly:"<<canvas;
    }    
    
    foreach(KisPaintingAssistant* assistant, d->assistants) {
            assistant->drawAssistant(gc, updateRect, converter, true, canvas, assistantVisibility(), outlineVisibility());
    }
    
}
//drawPreview//

QList<KisPaintingAssistantHandleSP> KisPaintingAssistantsDecoration::handles()
{
    QList<KisPaintingAssistantHandleSP> hs;
    foreach(KisPaintingAssistant* assistant, d->assistants) {
        foreach(const KisPaintingAssistantHandleSP handle, assistant->handles()) {
            if (!hs.contains(handle)) {
                hs.push_back(handle);
            }
        }
        foreach(const KisPaintingAssistantHandleSP handle, assistant->sideHandles()) {
            if (!hs.contains(handle)) {
                hs.push_back(handle);
            }
        }
    }
    return hs;
}

QList<KisPaintingAssistant*> KisPaintingAssistantsDecoration::assistants()
{
    return d->assistants;
}

void KisPaintingAssistantsDecoration::updateAction()
{
    d->toggleAssistant->setEnabled(!d->assistants.isEmpty());
    d->togglePreview->setEnabled(!d->assistants.isEmpty());
    d->toggleAssistant->setChecked(assistantVisibility());
    d->togglePreview->setChecked(outlineVisibility());
}

void KisPaintingAssistantsDecoration::setAssistantVisible(bool set)
{
    d->assistantVisible=set;
}

void KisPaintingAssistantsDecoration::setOutlineVisible(bool set)
{
    d->outlineVisible=set;
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
     foreach(KisPaintingAssistant* assistant, d->assistants) {
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
