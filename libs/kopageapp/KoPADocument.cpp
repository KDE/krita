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

#include "KoPADocument.h"

#include <kcommand.h>
#include <KTemporaryFile>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoSavingContext.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPAStyles.h"

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode )
{
    KoPAMasterPage * masterPage = new KoPAMasterPage();
    m_masterPages.append( masterPage );
    addPage( new KoPAPage( masterPage ), 0 /*add first*/ );
}

KoPADocument::~KoPADocument()
{
    qDeleteAll( m_pages );
    qDeleteAll( m_masterPages );
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect)
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}

bool KoPADocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    Q_UNUSED( doc );

    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                              const KoXmlDocument & settings, KoStore* store )
{
    Q_UNUSED( doc );
    Q_UNUSED( oasisStyles );
    Q_UNUSED( settings );
    Q_UNUSED( store  );
    return true;
}

bool KoPADocument::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    if ( !store->open( "content.xml" ) )
        return false;

    KoStoreDevice contentDev( store );
    KoXmlWriter* contentWriter = createOasisXmlWriter( &contentDev, "office:document-content" );

    KoGenStyles mainStyles;
    KoSavingContext savingContext( mainStyles, KoSavingContext::Store );

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter( &masterStyles, 1 );

    KoPASavingContext paContext( masterStylesTmpWriter, savingContext, 1 );

    paContext.setOptions( KoPASavingContext::DrawId | KoPASavingContext::AutoStyleInStyleXml );

    masterStylesTmpWriter.startElement( "office:master-styles" );

    // save master pages
    foreach( KoPAMasterPage *page, m_masterPages )
    {
        page->saveOdf( paContext );
    }
    masterStylesTmpWriter.endElement();

    masterStyles.close();

    // for office:body
    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );

    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( odfTagName() );

    paContext.setXmlWriter( contentTmpWriter );
    paContext.setOptions( KoPASavingContext::DrawId );

    // save pages
    QList<KoPAPage*>::const_iterator pageIt( m_pages.constBegin() );
    foreach ( KoPAPage *page, m_pages )
    {
        page->saveOdf( paContext );
        paContext.incrementPage();
    }

    contentTmpWriter.endElement(); // office:odfTagName()
    contentTmpWriter.endElement(); // office:body

    contentTmpFile.close();

    contentWriter->startElement( "office:automatic-styles" );
    saveOdfAutomaticStyles( *contentWriter, mainStyles, false );
    contentWriter->endElement();

    // And now we can copy over the contents from the tempfile to the real one
    contentWriter->addCompleteElement( &contentTmpFile );

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;

    if ( !store->close() ) // done with content.xml
        return false;

    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if ( !store->open( "styles.xml" ) )
        return false;

    manifestWriter->addManifestEntry( "styles.xml", "text/xml" );
    saveOdfDocumentStyles( store, mainStyles, &masterStyles );

    if ( !store->close() ) // done with styles.xml
        return false;

    return true;
}

void KoPADocument::saveOdfAutomaticStyles( KoXmlWriter& contentWriter, KoGenStyles& mainStyles, bool stylesDotXml )
{
    // test style writing
    QList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::STYLE_GRAPHICAUTO, stylesDotXml );
    QList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        ( *it ).style->writeStyle( &contentWriter, mainStyles, "style:style", ( *it ).name , "style:graphic-properties" );
    }

    styles = mainStyles.styles( KoPAStyles::STYLE_PAGE, stylesDotXml );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        ( *it ).style->writeStyle( &contentWriter, mainStyles, "style:style", ( *it ).name , "style:drawing-page-properties" );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_PAGELAYOUT, stylesDotXml );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        (*it).style->writeStyle( &contentWriter, mainStyles, "style:page-layout", (*it).name, "style:page-layout-properties" );
    }

}

void KoPADocument::saveOdfDocumentStyles( KoStore * store, KoGenStyles& mainStyles, QFile *masterStyles )
{
    KoStoreDevice stylesDev( store );
    KoXmlWriter* stylesWriter = createOasisXmlWriter( &stylesDev, "office:document-styles" );

    stylesWriter->startElement( "office:styles" );
    stylesWriter->endElement(); // office:styles

    stylesWriter->startElement( "office:automatic-styles" );
    saveOdfAutomaticStyles( *stylesWriter, mainStyles, true );
    stylesWriter->endElement(); // office:automatic-styles

    stylesWriter->addCompleteElement( masterStyles );

    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;
}

KoPAPage* KoPADocument::pageByIndex(int index)
{
    return m_pages.at(index);
}

void KoPADocument::addShape( KoShape * shape )
{
    if(!shape)
        return;

    KoShape * parent = shape;
    KoPAPageBase * page = 0;
    while ( !page && ( parent = parent->parent() ) )
    {
        page = dynamic_cast<KoPAPageBase*>( parent );
    }

    foreach( KoView *view, views() )
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        if ( page == kopaView->activePage() )
        {
            KoPACanvas *canvas = kopaView->kopaCanvas();
            canvas->shapeManager()->add( shape );
        }
    }
}


void KoPADocument::removeShape( KoShape *shape )
{
    if(!shape)
        return;

    KoShape * parent = shape;
    KoPAPageBase * page = 0;
    while ( !page && ( parent = parent->parent() ) )
    {
        page = dynamic_cast<KoPAPageBase*>( parent );
    }

    foreach( KoView *view, views() ) 
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );

        if ( page == kopaView->activePage() )    
        {
            KoPACanvas *canvas = kopaView->kopaCanvas();
            canvas->shapeManager()->remove( shape );
        }
    }
}

void KoPADocument::addPage(KoPAPage* page, KoPAPage* before)
{
    if(!page)
        return;

    int index = 0;

    if(before != 0)
        index = m_pages.indexOf(before);

    // Append the page if before wasn't found in m_pages
    if(index == -1)
        index = m_pages.count();

    m_pages.insert(index, page);
}

#include "KoPADocument.moc"
