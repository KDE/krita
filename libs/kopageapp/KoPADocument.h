/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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
class KoPAPageBase;
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

    /**
     * Get page by index.
     *
     * @param index of the page
     * @param masterPage if true return a masterPage, if false a normal page
     */
    KoPAPageBase* pageByIndex( int index, bool masterPage );

    /**
     * Insert @p page to the document after page @p before
     * @param page to insert to document
     * @param after the page which the inserted page should come after. Set after to 0 to add at the beginning
     */
    void insertPage( KoPAPageBase* page, KoPAPageBase* after );

    /**
     * Take @page from the page
     *
     * @param page taken from the document
     * @return the page taken form the document
     */
    KoPAPageBase * takePage( KoPAPageBase *page );

    void addShape( KoShape *shape );
    void removeShape( KoShape* shape );

protected:
    virtual KoView *createViewInstance( QWidget *parent ) = 0;
    virtual const char *odfTagName() = 0;

    void saveOdfAutomaticStyles( KoXmlWriter& contentWriter, KoGenStyles& mainStyles, bool stylesDotXml );
    void saveOdfDocumentStyles( KoStore * store, KoGenStyles& mainStyles, QFile *masterStyles );

private:
    QList<KoPAPageBase*> m_pages;
    QList<KoPAPageBase*> m_masterPages;
};

#endif /* KOPADOCUMENT_H */
