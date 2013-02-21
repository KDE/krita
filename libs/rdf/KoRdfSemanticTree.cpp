/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#include "KoRdfSemanticTree.h"
#include "KoDocumentRdf.h"
#include "KoRdfSemanticTreeWidgetItem.h"
#include <kdebug.h>
#include <klocale.h>
#include "KoRdfFoaF.h"
#include "KoRdfCalendarEvent.h"
#include "KoRdfLocation.h"

#include <QSet>

class KoRdfSemanticTreePrivate : public QSharedData
{
public:
    QTreeWidget *m_tree;
    QTreeWidgetItem *m_peopleItem;
    QTreeWidgetItem *m_eventsItem;
    QTreeWidgetItem *m_locationsItem;
    QList<hKoRdfFoaF> m_foafs;
    QList<hKoRdfCalendarEvent> m_cals;
    QList<hKoRdfLocation> m_locations;
    QList<hKoRdfLocation> m_tmp;

    KoRdfSemanticTreePrivate(QTreeWidget *tree)
        {
            m_tree = tree;
            if(m_tree)  {
                tree->sortItems(1, Qt::DescendingOrder);
                m_peopleItem = new QTreeWidgetItem(tree);
                m_peopleItem->setText(0, i18n("People"));
                tree->expandItem(m_peopleItem);
                m_eventsItem = new QTreeWidgetItem(tree);
                m_eventsItem->setText(0, i18n("Events"));
                tree->expandItem(m_eventsItem);
                m_locationsItem = new QTreeWidgetItem(tree);
                m_locationsItem->setText(0, i18n("Locations"));
                tree->expandItem(m_locationsItem);
            }
        }
    void update(KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> model);
    /**
     * Add the name of each selected child of parent to the 'ret' set.
     */
    void buildSelectedSet(QTreeWidgetItem *parent, QSet<QString> &ret);
    /**
     * Called from update() to reset the tree to a default state.
     */
    void clear(QTreeWidgetItem *parent);
};

KoRdfSemanticTree::KoRdfSemanticTree()
    : d(new KoRdfSemanticTreePrivate(0))
{
}

KoRdfSemanticTree::KoRdfSemanticTree(QTreeWidget *tree)
    : d(new KoRdfSemanticTreePrivate (tree))
{
}
KoRdfSemanticTree::KoRdfSemanticTree(const KoRdfSemanticTree &orig)
    : d(orig.d)
{
}

KoRdfSemanticTree::~KoRdfSemanticTree()
{
}

KoRdfSemanticTree KoRdfSemanticTree::createTree(QTreeWidget* tree)
{
    KoRdfSemanticTree ret (tree);
    return ret;
}

void KoRdfSemanticTreePrivate::clear(QTreeWidgetItem *parent)
{
    while (parent->childCount()) {
        QTreeWidgetItem* c = parent->child(0);
        m_peopleItem->removeChild(c);
        delete c;
    }
}

void KoRdfSemanticTreePrivate::buildSelectedSet(QTreeWidgetItem *parent, QSet<QString> &ret)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *c = parent->child(i);
        if (c->isSelected()) {
            if (KoRdfSemanticTreeWidgetItem *item = dynamic_cast<KoRdfSemanticTreeWidgetItem*>(c)) {
                ret << item->semanticItem()->name();
            }
        }
    }
}

void KoRdfSemanticTreePrivate::update(KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> model)
{
    QSet<QString> selectedPeople;
    QSet<QString> selectedEvents;
    QSet<QString> selectedLocations;

    buildSelectedSet(m_peopleItem, selectedPeople);
    buildSelectedSet(m_eventsItem, selectedEvents);
    buildSelectedSet(m_locationsItem, selectedLocations);
    clear(m_peopleItem);
    clear(m_eventsItem);
    clear(m_locationsItem);
    m_peopleItem->setSelected(false);
    m_eventsItem->setSelected(false);
    m_locationsItem->setSelected(false);
    m_foafs.clear();
    m_cals.clear();
    m_locations.clear();

    // people
    m_foafs = rdf->foaf(model);
    foreach (hKoRdfFoaF foaf, m_foafs) {
        KoRdfSemanticTreeWidgetItem *item = foaf->createQTreeWidgetItem(m_peopleItem);
        if (selectedPeople.contains(item->semanticItem()->name())) {
            item->setSelected(true);
        }
    }
    m_tree->expandItem(m_peopleItem);

    // events
    m_cals = rdf->calendarEvents(model);
    foreach (hKoRdfCalendarEvent e, m_cals) {
        KoRdfSemanticTreeWidgetItem *item = e->createQTreeWidgetItem(m_eventsItem);
        if (selectedEvents.contains(item->semanticItem()->name())) {
            item->setSelected(true);
        }
    }
    m_tree->expandItem(m_eventsItem);

   
    // locations
    if (model) {
        //
        // grab the lat/long triples from m_model into the passed model.
        //
        // geo84
        // <uri:dan84> <http://xmlns.com/foaf/0.1/based_near> _:genid1
        // _:genid1 <http://www.w3.org/2003/01/geo/wgs84_pos#lat> "51.47026" (empty)
        // _:genid1 <http://www.w3.org/2003/01/geo/wgs84_pos#long> "-2.59466" (empty)
        rdf->expandStatementsSubjectPointsTo(model);
        //kDebug(30015) << "expanding lists... old.sz:" << model->statementCount();
        // other geo is an rdf:list, so bring that in too
        rdf->expandStatementsToIncludeRdfLists(model);
        //kDebug(30015) << "expanding lists... new.sz:" << model->statementCount();
    }
    m_locations = rdf->locations(model);
    foreach (hKoRdfLocation e, m_locations) {
        KoRdfSemanticTreeWidgetItem *item = e->createQTreeWidgetItem(m_locationsItem);
        if (selectedLocations.contains(item->semanticItem()->name())) {
            item->setSelected(true);
        }
    }
    m_tree->expandItem(m_locationsItem);
}

void KoRdfSemanticTree::update(KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> model)
{
    d->update (rdf,model);
}


KoRdfSemanticTree &KoRdfSemanticTree::operator=(const KoRdfSemanticTree &other)
{
    d = other.d;
    return *this;
}
