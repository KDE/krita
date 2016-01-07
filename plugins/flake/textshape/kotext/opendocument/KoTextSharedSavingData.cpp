/* This file is part of the KDE project
Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>

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

#include "KoTextSharedSavingData.h"

#include "KoGenChanges.h"
#include "KoDocumentRdfBase.h"

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

#include <QMap>

class Q_DECL_HIDDEN KoTextSharedSavingData::Private
{
public:
    Private(void) : changes(0) { }
    ~Private() {}

    KoGenChanges *changes;
    QMap<QString, QString> m_rdfIdMapping; //< This lets the RDF system know old->new xml:id
#ifdef SHOULD_BUILD_RDF
    QSharedPointer<Soprano::Model> m_rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
#endif
    QMap<int, QString> styleIdToName;
};

KoTextSharedSavingData::KoTextSharedSavingData()
        : d(new Private())
{
}

KoTextSharedSavingData::~KoTextSharedSavingData()
{
}

void KoTextSharedSavingData::setGenChanges(KoGenChanges& changes) {
    d->changes = &changes;
}

KoGenChanges& KoTextSharedSavingData::genChanges() const
{
    return *(d->changes);
}

void KoTextSharedSavingData::addRdfIdMapping(const QString &oldid, const QString &newid)
{
    d->m_rdfIdMapping[ oldid ] = newid;
}

QMap<QString, QString> KoTextSharedSavingData::getRdfIdMapping() const
{
    return d->m_rdfIdMapping;
}

#ifdef SHOULD_BUILD_RDF
void KoTextSharedSavingData::setRdfModel(QSharedPointer<Soprano::Model> m)
{
    d->m_rdfModel = m;
}

QSharedPointer<Soprano::Model> KoTextSharedSavingData::rdfModel() const
{
    return d->m_rdfModel;
}
#endif

void KoTextSharedSavingData::setStyleName(int styleId, const QString &name)
{
    d->styleIdToName.insert(styleId, name);
}

QString KoTextSharedSavingData::styleName(int styleId) const
{
    return d->styleIdToName.value(styleId);
}

QList<QString> KoTextSharedSavingData::styleNames() const
{
    return d->styleIdToName.values();
}
