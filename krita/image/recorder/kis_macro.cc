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
#include "recorder/kis_recorded_action.h"
#include "recorder/kis_recorded_action_factory_registry.h"
#include "kis_undo_adapter.h"
#include "kis_play_info.h"
#include "kis_node_query_path.h"

struct KisMacro::Private {
    QList<KisRecordedAction*> actions;
};

KisMacro::KisMacro() : d(new Private)
{
}

KisMacro::KisMacro(const QList<KisRecordedAction*>& actions) : d(new Private)
{
    appendActions(actions);
}

KisMacro::~KisMacro()
{
    qDeleteAll(d->actions);
    delete d;
}


void KisMacro::appendActions(const QList<KisRecordedAction*>& actions)
{
    foreach(KisRecordedAction* action, actions) {
        addAction(*action);
    }
}

void KisMacro::removeActions(const QList<KisRecordedAction*>& actions)
{
    foreach(KisRecordedAction* action, actions) {
        d->actions.removeAll(action);
    }
    qDeleteAll(actions);
}

void KisMacro::addAction(const KisRecordedAction& action, const KisRecordedAction* before)
{
    if (before == 0) {
        KisRecordedAction* a = action.clone();
        Q_ASSERT(a);
        d->actions.append(a);
    } else {
        d->actions.insert(d->actions.indexOf(const_cast<KisRecordedAction*>(before)), action.clone());
    }
}

void KisMacro::moveAction(const KisRecordedAction* action, const KisRecordedAction* before)
{
    KisRecordedAction* _action = d->actions.takeAt(d->actions.indexOf(const_cast<KisRecordedAction*>(action)));
    if (before == 0) {
        Q_ASSERT(_action);
        d->actions.append(_action);
    } else {
        d->actions.insert(d->actions.indexOf(const_cast<KisRecordedAction*>(before)), _action);
    }
}

// TODO should be threaded instead
#include <QApplication>

void KisMacro::play(const KisPlayInfo& info) const
{
    dbgImage << "Start playing macro with " << d->actions.size() << " actions";
    if (info.undoAdapter()) {
        info.undoAdapter() ->beginMacro(i18n("Play macro"));
    }


    for (QList<KisRecordedAction*>::iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
        if (*it) {
            QList<KisNodeSP> nodes = (*it)->nodeQueryPath().queryNodes(info.image(), info.currentNode());
            foreach(const KisNodeSP node, nodes) {
                dbgImage << "Play action : " << (*it)->name();
                (*it)->play(node, info);
            }
        }
        QApplication::processEvents();
    }

    if (info.undoAdapter()) {
        info.undoAdapter() ->endMacro();
    }
}

void KisMacro::fromXML(const QDomElement& docElem)
{
    d->actions.clear();
    QDomNode node = docElem.firstChild();
    while (!node.isNull()) {
        QDomElement elt = node.toElement(); // try to convert the node to an element.
        if (!elt.isNull() && elt.tagName() == "RecordedAction") {
            QString id = elt.attribute("id", "");
            if (!id.isNull()) {
                dbgImage << "Reconstruct : " << id << endl; // the node really is an element.
                KisRecordedActionFactory* raf = KisRecordedActionFactoryRegistry::instance()->get(id);
                if (raf) {
                    KisRecordedAction* a = raf->fromXML(elt);
                    Q_ASSERT(a);
                    d->actions.append(a); // TODO should use addAction
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
    for (QList<KisRecordedAction*>::iterator it = d->actions.begin();
            it != d->actions.end(); ++it) {
        QDomElement eAct = doc.createElement("RecordedAction");
        (*it)->toXML(doc, eAct);
        e.appendChild(eAct);
    }
}

const QList<KisRecordedAction*>& KisMacro::actions() const
{
    return d->actions;
}

#include "kis_macro.moc"
