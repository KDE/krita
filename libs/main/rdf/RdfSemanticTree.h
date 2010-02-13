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

#ifndef __rdf_RdfSemanticTree_h__
#define __rdf_RdfSemanticTree_h__

#include "komain_export.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "rdf/RdfFoaF.h"
#include "rdf/RdfCalendarEvent.h"
#include "rdf/RdfLocation.h"

/**
 * @short Manage a QTreeWidget showing a collection of Semantic Items
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * Because a tree viewing people, places, events etc "Semantic Objects"
 * is created and updated in multiple places, it makes sense to collect
 * the code handling it into a central location for easy global updates.
 */
struct KOMAIN_EXPORT RdfSemanticTree {
    QTreeWidget* m_tree;
    QTreeWidgetItem* m_peopleItem;
    QTreeWidgetItem* m_eventsItem;
    QTreeWidgetItem* m_locationsItem;
    QList<RdfFoaF*> m_foafs;
    QList<RdfCalendarEvent*> m_cals;
    QList<RdfLocation*> m_locations;

    /**
     * Setup the TreeWidget with a default sorting order etc and
     * initialize the people, events and locations base tree Items.
     */
    static RdfSemanticTree createTree(QTreeWidget* v);

    /**
     * Update the Items shown in the tree to reflect the Rdf in the
     * given model. if the model is not passed in then any Rdf in the
     * KoDocumentRdf is used to populate the tree.
     */
    void update(KoDocumentRdf* rdf, Soprano::Model* model = 0);
private:

    /**
     * Called from update() to reset the tree to a default state.
     */
    void clear(QTreeWidgetItem* parent);

    /**
     * Add the name of each selected child of parent to the 'ret' set.
     */
    QSet<QString>& buildSelectedSet(QTreeWidgetItem* parent, QSet<QString>& ret);

};

#endif


