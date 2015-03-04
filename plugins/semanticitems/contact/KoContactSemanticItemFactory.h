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

#ifndef KOCONTACTSEMANTICITEMFACTORY_H
#define KOCONTACTSEMANTICITEMFACTORY_H

#include <KoRdfSemanticItemFactoryBase.h>

class KoContactSemanticItemFactory : public KoRdfSemanticItemFactoryBase
{
public:
    KoContactSemanticItemFactory();

public: // KoRdfSemanticItemFactoryBase API
    virtual QString className() const;
    virtual QString classDisplayName() const;

    virtual void updateSemanticItems(QList<hKoRdfBasicSemanticItem> &semanticItems, const KoDocumentRdf *rdf, QSharedPointer<Soprano::Model> m);
    virtual hKoRdfBasicSemanticItem createSemanticItem(const KoDocumentRdf *rdf, QObject *parent);
    virtual bool canCreateSemanticItemFromMimeData(const QMimeData *mimeData) const;
    virtual hKoRdfBasicSemanticItem createSemanticItemFromMimeData(const QMimeData* mimeData, KoCanvasBase* host,
                                                              const KoDocumentRdf *rdf, QObject *parent = 0) const;
    virtual bool isBasic() const;
};

#endif
