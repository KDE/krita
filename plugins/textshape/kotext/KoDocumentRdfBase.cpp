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

#include "KoDocumentRdfBase.h"

#include <KoDocumentResourceManager.h>
#include <KoText.h>

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

#include "TextDebug.h"

KoDocumentRdfBase::KoDocumentRdfBase(QObject *parent)
        : QObject(parent)
{
}

KoDocumentRdfBase::~KoDocumentRdfBase()
{
}

#ifdef SHOULD_BUILD_RDF
QSharedPointer<Soprano::Model> KoDocumentRdfBase::model() const
{
    return QSharedPointer<Soprano::Model>(0);
}
#endif

void KoDocumentRdfBase::linkToResourceManager(KoDocumentResourceManager *rm)
{
    QVariant variant;
    variant.setValue<QObject*>(this);
    rm->setResource(KoText::DocumentRdf, variant);
}

void KoDocumentRdfBase::updateInlineRdfStatements(const QTextDocument *qdoc)
{
    Q_UNUSED(qdoc);
}

void KoDocumentRdfBase::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    Q_UNUSED(m);
}

bool KoDocumentRdfBase::loadOasis(KoStore *store)
{
    Q_UNUSED(store);
    return true;
}

bool KoDocumentRdfBase::saveOasis(KoStore *store, KoXmlWriter *manifestWriter)
{
    Q_UNUSED(store);
    Q_UNUSED(manifestWriter);
    return true;
}

bool KoDocumentRdfBase::completeLoading(KoStore */*store*/)
{
    return false;
}

bool KoDocumentRdfBase::completeSaving(KoStore */*store*/, KoXmlWriter */*manifestWriter*/, KoShapeSavingContext */*context*/)
{
    return false;
}

QStringList KoDocumentRdfBase::idrefList() const
{
    return QStringList();
}
