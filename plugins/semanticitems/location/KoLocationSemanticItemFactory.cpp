/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2013 Friedrich W. H. Kossebau <kossebau@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoLocationSemanticItemFactory.h"

// lib
#include "KoRdfLocation.h"
// KDE
#include <kdebug.h>
#include <klocale.h>


KoLocationSemanticItemFactory::KoLocationSemanticItemFactory()
: KoRdfSemanticItemFactoryBase("Location")
{
}

KoLocationSemanticItemFactory::~KoLocationSemanticItemFactory()
{
}

QString KoLocationSemanticItemFactory::className() const
{
    return QLatin1String("Location");
}

QString KoLocationSemanticItemFactory::classDisplayName() const
{
    return i18nc("displayname of the semantic item type Location", "Location");
}

/**
 * The redland library is used for in memory Rdf by Soprano. Unfortunately
 * the distinct keyword doesn't always do what it should so a postprocess
 * has to be applied in some cases to ensure DISTINCT semantics in the results.
 */
struct SparqlDistinctPostprocess {
    explicit SparqlDistinctPostprocess(const QString &bindingForID) {
        m_bindingsThatMakeID << bindingForID;
    }
    bool shouldSkip(const Soprano::QueryResultIterator &it) {
        const QString id = uniqueID(it);
        const bool ret = m_uniqfilter.contains(id);
        m_uniqfilter << id;
        return ret;
    }
    void addBindingToKeySet(const QString &n) {
        m_bindingsThatMakeID << n;
    }

protected:
    QString uniqueID(const Soprano::QueryResultIterator &it) {
        QString ret;

        foreach (const QString &b, m_bindingsThatMakeID) {
            ret += it.binding(b).toString();
        }

        return ret;
    }
private:
    QStringList m_bindingsThatMakeID;
    QSet<QString> m_uniqfilter;
};


/**
 * Because there are at least two different ways of associating digital longitude
 * and latitude with triples in Rdf, the locations() method farms off discovering
 * these values to this method using specific SPARQL query text.
 */
static void addLocations(QList<hKoRdfSemanticItem> &ret, const KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> m,
                         bool isGeo84,
                         const QString &sparqlQuery)
{
    Soprano::QueryResultIterator it = m->executeQuery(sparqlQuery,
                        Soprano::Query::QueryLanguageSparql);
    SparqlDistinctPostprocess uniqFilter(QLatin1String("lat"));
    uniqFilter.addBindingToKeySet(QLatin1String("long"));
    while (it.next()) {
        if (uniqFilter.shouldSkip(it)) {
            continue;
        }

        hKoRdfSemanticItem semObj(new KoRdfLocation(0, rdf, it, isGeo84));
        ret << semObj;
    }
}


void KoLocationSemanticItemFactory::updateSemanticItems(QList<hKoRdfBasicSemanticItem> &semanticItems, const KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> m)
{
    QList<hKoRdfSemanticItem> currentKoRdfLocations;
    addLocations(currentKoRdfLocations, rdf, m, false,
        "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        "prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#> \n"
        "select distinct ?graph ?geo ?long ?lat ?joiner \n"
        "where {  \n"
        " GRAPH ?graph { \n"
        "              ?ev cal:geo ?geo . \n"
        "              ?geo rdf:first ?lat . \n"
        "              ?geo rdf:rest ?joiner . \n"
        "              ?joiner rdf:first ?long \n"
        "              } \n"
        "} \n");
    addLocations(currentKoRdfLocations, rdf, m, true,
        "prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
        "select ?graph ?geo ?long ?lat ?type \n"
        "where {  \n"
        " GRAPH ?graph { \n"
        " \n"
        "       ?geo geo84:lat  ?lat . \n"
        "       ?geo geo84:long ?long \n"
        "       OPTIONAL { ?geo rdf:type ?type } \n"
        " \n"
        " } \n"
        "} \n");

    // add the new, remove the no longer existing between locObjects and currentKoRdfLocations.
    // The semantic items have a lifetime of this KoDocumentRDF.
    // If we could use smart pointers then we could just return the new list of locations,
    // As semantic items have a lifetime of this KoDocumentRDF,
    // we don't want to create any more than are needed.
    //
    // As currentKoRdfLocations contains all the location semitems we have found to be valid,
    // we need to transfer any new ones from that list to locObjects and delete what
    // remains (which are objects that existed in locObjects before and were rediscovered
    // during the query process).
    //
    // Creating a list of locations each time similifies the query and discovery process
    // at the expense of this little mess to merge the new and old with explicit pointer
    // and object lifetime handling
    QList<hKoRdfBasicSemanticItem> removeSet;
    foreach (hKoRdfBasicSemanticItem oldItem, semanticItems) {
        const QString oldItemLs = oldItem->linkingSubject().toString();
        bool found = false;
        foreach (hKoRdfBasicSemanticItem newItem, currentKoRdfLocations) {
            if (oldItemLs == newItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeSet << oldItem;
        }
    }

    foreach (hKoRdfBasicSemanticItem item, removeSet) {
        semanticItems.removeAll(item);
    }

    foreach (hKoRdfBasicSemanticItem newItem, currentKoRdfLocations) {
        const QString newItemLs = newItem->linkingSubject().toString();
        bool found = false;
        foreach (hKoRdfBasicSemanticItem oldItem, semanticItems) {
            if (newItemLs == oldItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            semanticItems << newItem;
        }
    }
}

hKoRdfBasicSemanticItem KoLocationSemanticItemFactory::createSemanticItem(const KoDocumentRdf* rdf, QObject* parent)
{
    return hKoRdfBasicSemanticItem(new KoRdfLocation(parent, rdf));
}

bool KoLocationSemanticItemFactory::canCreateSemanticItemFromMimeData(const QMimeData* mimeData) const
{
    Q_UNUSED(mimeData);
    return false;
}

hKoRdfBasicSemanticItem KoLocationSemanticItemFactory::createSemanticItemFromMimeData(const QMimeData* mimeData, KoCanvasBase* host, const KoDocumentRdf* rdf, QObject* parent) const
{
    Q_UNUSED(mimeData);
    Q_UNUSED(host);
    Q_UNUSED(rdf);
    Q_UNUSED(parent);
    return hKoRdfSemanticItem(0);
}

bool KoLocationSemanticItemFactory::isBasic() const
{
    return false;
}
