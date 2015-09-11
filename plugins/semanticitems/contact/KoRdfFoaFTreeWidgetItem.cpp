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

#include "KoRdfFoaFTreeWidgetItem.h"
#include "KoDocumentRdf.h"
#include "RdfSemanticTreeWidgetSelectAction.h"

#include <kdebug.h>
#include <klocalizedstring.h>


KoRdfFoaFTreeWidgetItem::KoRdfFoaFTreeWidgetItem(QTreeWidgetItem *parent, hKoRdfFoaF foaf)
        : KoRdfSemanticTreeWidgetItem(parent)
        , m_foaf(foaf)
{
    setText(ColName, m_foaf->name());
}

hKoRdfSemanticItem KoRdfFoaFTreeWidgetItem::semanticItem() const
{
    kDebug(30015) << "ret. m_foaf:" << m_foaf;
    return m_foaf;
}

QString KoRdfFoaFTreeWidgetItem::uIObjectName() const
{
    return i18n("Contact Information");
}

hKoRdfFoaF KoRdfFoaFTreeWidgetItem::foaf() const
{
    return m_foaf;
}

void KoRdfFoaFTreeWidgetItem::insert(KoCanvasBase *host)
{
    foaf()->insert(host);
}

QList<QAction *> KoRdfFoaFTreeWidgetItem::actions(QWidget *parent, KoCanvasBase *host)
{
    QList<QAction *> m_actions;
    QAction *action = 0;

    action = createAction(parent, host, i18n("Edit..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(edit()));
    m_actions.append(action);
    action = createAction(parent, host, i18n("Import contact"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(importSelectedSemanticViewContact()));
    m_actions.append(action);
    action = createAction(parent, host, i18n("Export as vcard..."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(exportToFile()));
    m_actions.append(action);
    addApplyStylesheetActions(parent, m_actions, host);
    if (host) {
        action = new RdfSemanticTreeWidgetSelectAction(parent, host, semanticItem());
        m_actions.append(action);
    }
    return m_actions;
}

void KoRdfFoaFTreeWidgetItem::importSelectedSemanticViewContact()
{
    foaf()->saveToKABC();
}

void KoRdfFoaFTreeWidgetItem::exportToFile()
{
    foaf()->exportToFile();
}
