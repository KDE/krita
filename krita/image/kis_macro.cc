/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_macro.h"
#include <QDomNode>

#include <kis_debug.h>
#include <klocale.h>

#include "kis_image.h"
#include "kis_recorded_action.h"
#include "kis_recorded_action_factory_registry.h"
#include "kis_undo_adapter.h"

struct KisMacro::Private {
    QList<KisRecordedAction*> actions;
    KisImageSP image;
};

KisMacro::KisMacro(KisImageSP _image) : d(new Private)
{
    d->image = _image;
}

KisMacro::KisMacro(KisImageSP _image, const QList<KisRecordedAction*>& actions) : d(new Private)
{
    d->image = _image;
    appendActions(actions);
}

void KisMacro::appendActions(const QList<KisRecordedAction*>& actions)
{
    foreach(KisRecordedAction* action, actions)
    {
        d->actions.append(action->clone());
    }
}

void KisMacro::addAction(const KisRecordedAction& action)
{
    d->actions.append(action.clone());
}

#include <QApplication>

void KisMacro::play()
{
    dbgImage << "Start playing macro with " << d->actions.size() << " actions";
    KisUndoAdapter* undoAdapter = 0;
    if(d->image->undo())
    {
        undoAdapter = d->image->undoAdapter();
        undoAdapter->beginMacro(i18n("Play macro"));
    }
    
    for( QList<KisRecordedAction*>::iterator it = d->actions.begin();
         it != d->actions.end(); ++it)
    {
        dbgImage << "Play action : " << (*it)->name();
        (*it)->play(undoAdapter);
        QApplication::processEvents();
    }
    
    if(undoAdapter)
    {
        undoAdapter->endMacro();
    }
}

void KisMacro::fromXML(const QDomElement& docElem)
{
    d->actions.clear();
    QDomNode node = docElem.firstChild();
    while(!node.isNull()) {
        QDomElement elt = node.toElement(); // try to convert the node to an element.
        if(!elt.isNull() && elt.tagName() == "RecordedAction") {
            QString id = elt.attribute("id", "");
            if(!id.isNull())
            {
                dbgImage << "Reconstruct : " << id << endl; // the node really is an element.
                KisRecordedActionFactory* raf = KisRecordedActionFactoryRegistry::instance()->get(id);
                if(raf)
                {
                    d->actions.append( raf->fromXML( d->image, elt) );
                } else {
                    dbgImage << "Unknown action : " << id << endl;
                }
            } else {
                dbgImage << "Invalid recorded action: null id";
            }
        } else {
            dbgImage << "Unknown element " << elt.tagName() << (elt.tagName() == "RecordedAction");
        }
        node = node.nextSibling();
    }
}

void KisMacro::toXML(QDomDocument& doc, QDomElement& e) const
{
    for( QList<KisRecordedAction*>::iterator it = d->actions.begin();
        it != d->actions.end(); ++it)
    {
        QDomElement eAct = doc.createElement("RecordedAction");
        (*it)->toXML(doc, eAct);
        e.appendChild(eAct);
    }
}

#include "kis_macro.moc"
