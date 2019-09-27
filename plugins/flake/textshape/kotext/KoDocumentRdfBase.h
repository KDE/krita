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

#ifndef KO_DOCUMENT_Rdf_Base_H
#define KO_DOCUMENT_Rdf_Base_H

#include "kritatext_export.h"

#include <QObject>
#include <QMap>
#include <QString>
#include <QMetaType>
#include <QSharedPointer>

#include <KoDataCenterBase.h>

class KoDocumentResourceManager;
class QTextDocument;
class KoStore;
class KoXmlWriter;


///**
// * Dummy definition in case Soprano is not available.
// */
namespace Soprano
{
    class Model;
}

/**
 * A base class that provides the interface to many RDF features
 * but will not do anything if Soprano support is not built.
 * By having this "Base" class, code can call methods at points
 * where RDF handling is desired and can avoid \#ifdef conditionals
 * because the base class interface is here and will be valid, even
 * if impotent when Soprano support is not built.
 */
class KRITATEXT_EXPORT KoDocumentRdfBase : public QObject, public KoDataCenterBase
{
    Q_OBJECT

public:
    explicit KoDocumentRdfBase(QObject *parent = 0);
    ~KoDocumentRdfBase() override;

    /**
     * Get the Soprano::Model that contains all the Rdf
     * You do not own the model, do not delete it.
     */
#ifdef SHOULD_BUILD_RDF
    virtual QSharedPointer<Soprano::Model> model() const;
#endif
    virtual void linkToResourceManager(KoDocumentResourceManager *rm);

    virtual void updateInlineRdfStatements(const QTextDocument *qdoc);
    virtual void updateXmlIdReferences(const QMap<QString, QString> &m);

    /**
     * idrefList queries soprano after loading and creates a list of all rdfid's that
     * where found in the manifest.rdf document. This list is used to make sure we do not
     * create more inline rdf objects than necessary
     * @return a list of xml-id's
     */
    virtual QStringList idrefList() const;


    virtual bool loadOasis(KoStore *store);
    virtual bool saveOasis(KoStore *store, KoXmlWriter *manifestWriter);

    // reimplemented in komain/rdf/KoDocumentRdf
    bool completeLoading(KoStore *store) override;
    bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context) override;
};

Q_DECLARE_METATYPE(KoDocumentRdfBase*)

#endif

