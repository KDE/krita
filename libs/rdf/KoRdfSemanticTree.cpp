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

#include "KoRdfSemanticItemRegistry.h"
#include "KoDocumentRdf.h"
#include "KoRdfSemanticTreeWidgetItem.h"
// KDE
#include <kdebug.h>
#include <klocale.h>
// Qt
#include <QSet>

class KoRdfSemanticTreePrivate : public QSharedData
{
public:
    QTreeWidget *m_tree;
    QHash<QString, QTreeWidgetItem*> m_treeWidgetItems;

    KoRdfSemanticTreePrivate(QTreeWidget *tree)
        {
            m_tree = tree;
            if(m_tree)  {
                tree->sortItems(1, Qt::DescendingOrder);

                foreach (const QString &semanticClass, KoRdfSemanticItemRegistry::instance()->classNames()) {
                    QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem(tree);
                    treeWidgetItem->setText(0, KoRdfSemanticItemRegistry::instance()->classDisplayName(semanticClass));
                    tree->expandItem(treeWidgetItem);
                    m_treeWidgetItems.insert(semanticClass, treeWidgetItem);
                }
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
        parent->removeChild(c);
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
    QHash<QString, QTreeWidgetItem*>::ConstIterator it = m_treeWidgetItems.constBegin();
    QHash<QString, QTreeWidgetItem*>::ConstIterator end = m_treeWidgetItems.constEnd();
    for( ; it != end; ++it) {
        const QString &semanticClass = it.key();
        QTreeWidgetItem *treeWidgetItem = it.value();

        QSet<QString> selectedItems;
        buildSelectedSet(treeWidgetItem, selectedItems);
        clear(treeWidgetItem);
        treeWidgetItem->setSelected(false);

        // TODO: for locations this unexplained hack is done, find out why and if really needed
        if (model && semanticClass == QLatin1String("Location")) {
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

        const QList<hKoRdfSemanticItem> semanticItems = KoRdfSemanticItem::fromList(rdf->semanticItems(semanticClass, model));
        foreach (hKoRdfSemanticItem semanticItem, semanticItems) {
            KoRdfSemanticTreeWidgetItem *item = semanticItem->createQTreeWidgetItem(treeWidgetItem);
            if (!item) { //FIXME: add this cuz rdf info dialog is crashing. I don't want to implement QTreeWidgetItem for AuthorSection
                continue;
            }

            if (selectedItems.contains(item->semanticItem()->name())) {
                item->setSelected(true);
            }
        }
        m_tree->expandItem(treeWidgetItem);
    }
}

void KoRdfSemanticTree::update(KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> model)
{
    d->update(rdf,model);
}


KoRdfSemanticTree &KoRdfSemanticTree::operator=(const KoRdfSemanticTree &other)
{
    d = other.d;
    return *this;
}
