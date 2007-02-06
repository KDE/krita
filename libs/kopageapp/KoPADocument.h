/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPADOCUMENT_H
#define KOPADOCUMENT_H

#include <QObject>

#include <KoDocument.h>
#include <KoShapeControllerBase.h>
#include "kopageapp_export.h"

class KoPAPage;
class KoPAMasterPage;

/// Document class that stores KoPAPage and KoPAMasterPage objects
class KOPAGEAPP_EXPORT KoPADocument : public KoDocument, public KoShapeControllerBase
{
    Q_OBJECT
public:
    explicit KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode = false );
    virtual ~KoPADocument();

    void paintContent( QPainter &painter, const QRect &rect);

    bool loadXML( QIODevice *, const KoXmlDocument & doc );
    bool loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                    const KoXmlDocument & settings, KoStore* store );

    bool saveOasis( KoStore* store, KoXmlWriter* manifestWriter );

    KoPAPage* pageByIndex(int index);

    /**
     * Add @p page to the document after page @p before
     * @param page page to add to document
     * @param before the page which the added page should come after. Set before to 0 to add at the beginning
     */
    void addPage(KoPAPage* page, KoPAPage* before);

    void addShape( KoShape *shape );
    void removeShape( KoShape* shape );

protected:
    virtual KoView *createViewInstance(  QWidget *parent ) = 0;

private:
    QList<KoPAPage*> m_pages;
    QList<KoPAMasterPage*> m_masterPages;
};

#endif /* KOPADOCUMENT_H */
