/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2013 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KORDFSEMANTICITEMFACTORYBASE_H
#define KORDFSEMANTICITEMFACTORYBASE_H

#include <kordf_export.h>

// Calligra
#include <KoRdfSemanticItem.h>

// Qt
#include <QString>

class QMimeData;

/**
 * A factory for semantic item objects. There should be one for each semantic item class to
 * allow the creation of such items from that plugin.
 * @see KoRdfSemanticItemRegistry
 */
class KORDF_EXPORT KoRdfSemanticItemFactoryBase
{
public:
    /**
     * Create the new factory
     * @param parent the parent QObject for memory management usage.
     * @param id a string that will be used internally for referencing the semantic item type.
     */
    explicit KoRdfSemanticItemFactoryBase(const QString &id);
    virtual ~KoRdfSemanticItemFactoryBase();

public: // API required by KoGenericRegistry
    /**
     * return the id for the semantic item this factory creates.
     * @return the id for the semantic item this factory creates.
     */
    QString id() const;

public: // API to be implemented
    /**
     * Returns the name of the class of semantic items this factory creates.
     */
    virtual QString className() const = 0;

    /**
     * Returns the display name of the class of semantic items this factory creates.
     */
    virtual QString classDisplayName() const = 0;

    virtual void updateSemanticItems(QList<hKoRdfBasicSemanticItem> &semanticItems, const KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> m) = 0;

    /**
     * Create a new instance of a semantic item.
     */
    virtual hKoRdfBasicSemanticItem createSemanticItem(const KoDocumentRdf *rdf, QObject *parent) = 0;

    /**
     * Returns if a semantic item could be principially created from the passed mimeData.
     * Creation could still fail if actually tried.
     */
    virtual bool canCreateSemanticItemFromMimeData(const QMimeData *mimeData) const = 0;

    /**
     * Create a new instance of a semantic item from the passed mimeData. Returns 0 if that fails.
     */
    virtual hKoRdfBasicSemanticItem createSemanticItemFromMimeData(const QMimeData* mimeData, KoCanvasBase *host,
                                                                   const KoDocumentRdf *rdf, QObject *parent = 0) const = 0;

    virtual bool isBasic() const = 0;
private:
    class Private;
    Private * const d;
};

#endif
