/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoRdfCalendarEventTreeWidgetItem.h"

#include "KoDocumentRdf.h"
#include "RdfSemanticTreeWidgetSelectAction.h"

#include <kdebug.h>
#include <klocalizedstring.h>

KoRdfCalendarEventTreeWidgetItem::KoRdfCalendarEventTreeWidgetItem(QTreeWidgetItem* parent,
                                                                   hKoRdfCalendarEvent ev)
        : KoRdfSemanticTreeWidgetItem(parent)
        , m_semanticObject(ev)
{
    setText(ColName, m_semanticObject->name());
}

hKoRdfSemanticItem KoRdfCalendarEventTreeWidgetItem::semanticItem() const
{
    return m_semanticObject;
}

QString KoRdfCalendarEventTreeWidgetItem::uIObjectName() const
{
    return i18n("Calendar Event");
}

QList<QAction *> KoRdfCalendarEventTreeWidgetItem::actions(QWidget *parent, KoCanvasBase* host)
{
    QList<QAction *> m_actions;
    QAction* action = 0;
    action = createAction(parent, host, i18n("Edit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(edit()));
    m_actions.append(action);
    action = createAction(parent, host, i18n("Import event to Calendar"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(saveToKCal()));
    m_actions.append(action);
    action = createAction(parent, host, i18n("Export event to iCal file..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(exportToFile()));
    m_actions.append(action);
    addApplyStylesheetActions(parent, m_actions, host);
    if (host) {
        action = new RdfSemanticTreeWidgetSelectAction(parent, host, semanticItem());
        m_actions.append(action);
    }
    return m_actions;
}

hKoRdfCalendarEvent KoRdfCalendarEventTreeWidgetItem::semanticObject() const
{
    return m_semanticObject;
}

void KoRdfCalendarEventTreeWidgetItem::insert(KoCanvasBase *host)
{
    semanticObject()->insert(host);
}

void KoRdfCalendarEventTreeWidgetItem::saveToKCal()
{
    kDebug(30015) << "import a calendar event from the document... "
        << " name:" << m_semanticObject->name();
    semanticObject()->saveToKCal();
}

void KoRdfCalendarEventTreeWidgetItem::exportToFile()
{
    kDebug(30015) << "exporting to an iCal file..."
        << " name:" << m_semanticObject->name();
    semanticObject()->exportToFile();
}
