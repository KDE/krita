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

#ifndef KORDFSEMANTICITEMREGISTRY_H
#define KORDFSEMANTICITEMREGISTRY_H

#include "kordf_export.h"

// Calligra
#include "KoRdfSemanticItem.h"
#include "KoRdfSemanticItemFactoryBase.h"
#include <KoGenericRegistry.h>

class KoCanvasBase;

/**
 * This singleton class keeps a register of all available semantic item factories.
 * @see KoRdfSemanticItemFactoryBase
 * @see KoRdfSemanticItem
 */
class KORDF_EXPORT KoRdfSemanticItemRegistry : public KoGenericRegistry<KoRdfSemanticItemFactoryBase*>
{
public:
    ~KoRdfSemanticItemRegistry();

    /**
     * Return an instance of the KoRdfSemanticItemRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoRdfSemanticItemRegistry *instance();

    /**
     * Gets a list of SemanticItem subclasses that can be created.
     * Any of the strings in the return value can be created using
     * createSemanticItem().
     *
     * @see createSemanticItem()
     */
    QStringList classNames() const;

    /**
     * Returns the display name of the class, or an empty string if none was found.
     */
    QString classDisplayName(const QString &className) const;

    /**
     * Create a SemanticItem subclass using its name from
     * classNames(). Useful for menus and other places that want to
     * allow the user to create new SemanticItem Objects.
     */
    hKoRdfBasicSemanticItem createSemanticItem(const QString &semanticClass, const KoDocumentRdf *docRdf, QObject *parent = 0) const;

    /**
     * Create a SemanticItem subclass from the passed mimeData.
     * TODO: support that mimedata could be used for different semantic item classes
     */
    hKoRdfBasicSemanticItem createSemanticItemFromMimeData(const QMimeData *mimeData, KoCanvasBase *host, const KoDocumentRdf *docRdf, QObject *parent = 0) const;

    /**
     * Returns if a semantic item could be principially created from the passed mimeData.
     * Creation could still fail if actually tried.
     */
    bool canCreateSemanticItemFromMimeData(const QMimeData *mimeData) const;

    void updateSemanticItems(
        QList<hKoRdfBasicSemanticItem> &semanticItems,
        const KoDocumentRdf *docRdf,
        const QString &className,
        QSharedPointer<Soprano::Model> m = QSharedPointer<Soprano::Model>(0)) const;

    bool isBasic(const QString &className);

private:
    KoRdfSemanticItemRegistry();
    KoRdfSemanticItemRegistry(const KoRdfSemanticItemRegistry&);
    KoRdfSemanticItemRegistry operator=(const KoRdfSemanticItemRegistry&);

    class Private;
    Private * const d;
};

#endif
