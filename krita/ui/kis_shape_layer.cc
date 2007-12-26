/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_shape_layer.h"

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QDomElement>
#include <QDomDocument>
#include <QIcon>
#include <QString>
#include <QList>

#include <ktemporaryfile.h>
#include <kicon.h>

#include <KoCompositeOp.h>
#include <KoDocument.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoOasisLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoOdfWriteStore.h>
#include <KoPageLayout.h>
#include <KoShapeContainer.h>
#include <KoShapeLayer.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoShapeSavingContext.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoShapeContainer.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_shape_layer_canvas.h"
#include "kis_image_view_converter.h"
#include <kis_painter.h>
#include "kis_node_visitor.h"

class KisShapeLayer::Private
{
public:
    KoViewConverter * converter;
    qint32 x;
    qint32 y;
    KisPaintDeviceSP projection;
    KisShapeLayerCanvas * canvas;
};

KisShapeLayer::KisShapeLayer( KoShapeContainer * parent,
                              KisImageSP img,
                              const QString &name,
                              quint8 opacity )
    : KisExternalLayer( img, name, opacity )
    , m_d( new Private() )
{
    KoShapeContainer::setParent( parent );
    setShapeId( KIS_SHAPE_LAYER_ID );

    m_d->converter = new KisImageViewConverter(image());
    m_d->x = 0;
    m_d->y = 0;
    m_d->projection = new KisPaintDevice( img->colorSpace() );
    m_d->canvas = new KisShapeLayerCanvas( this, m_d->converter );
    m_d->canvas->setProjection( m_d->projection );
}

KisShapeLayer::~KisShapeLayer()
{
    delete m_d->converter;
    delete m_d->canvas;
    delete m_d;
}

bool KisShapeLayer::allowAsChild( KisNodeSP node) const
{
    if ( node->inherits( "KisMask" ) )
       return true;
    else
        return false;
}

void KisShapeLayer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( converter );
    Q_UNUSED( painter );
}

void KisShapeLayer::addChild(KoShape *object)
{
    kDebug(41001) <<"KisShapeLayer::addChild";
    KoShapeLayer::addChild( object );
    m_d->canvas->shapeManager()->add( object );

    setDirty(m_d->converter->documentToView(object->boundingRect()).toRect());
}

void KisShapeLayer::removeChild(KoShape *object)
{
    m_d->canvas->shapeManager()->remove( object );
    KoShapeLayer::removeChild( object );
}

QIcon KisShapeLayer::icon() const
{
    return KIcon("gear");
}

void KisShapeLayer::updateProjection(const QRect& r)
{
    kDebug(41001) <<"KisShapeLayer::updateProjection()" << r;
}

KisPaintDeviceSP KisShapeLayer::projection() const
{
    return m_d->projection;
}

qint32 KisShapeLayer::x() const
{
    return m_d->x;
}

void KisShapeLayer::setX(qint32 x)
{
    if ( x == m_d->x ) return;
    m_d->x = x;
    setDirty();
}

qint32 KisShapeLayer::y() const
{
    return m_d->y;
}

void KisShapeLayer::setY(qint32 y)
{
    if ( y == m_d->y ) return;
    m_d->y = y;
    setDirty();
}

QRect KisShapeLayer::extent() const
{
    QRect rc = boundingRect().toRect();
    return QRectF( rc.x() * image()->xRes(), rc.y() * image()->yRes(), rc.width() * image()->xRes(), rc.height() * image()->yRes() ).toRect();
}

QRect KisShapeLayer::exactBounds() const
{
    QRect rc = boundingRect().toRect();
    return QRectF( rc.x() * image()->xRes(), rc.y() * image()->yRes(), rc.width() * image()->xRes(), rc.height() * image()->yRes() ).toRect();
}

bool KisShapeLayer::accept(KisNodeVisitor& visitor)
{
    return visitor.visit(this);
}

KoShapeManager *KisShapeLayer::shapeManager() const
{
    return m_d->canvas->shapeManager();
}

bool KisShapeLayer::saveOdf(KoStore * store) const
{

    QList<KoShape*> shapes = iterator();
    qSort( shapes.begin(), shapes.end(), KoShape::compareShapeZIndex );

    foreach(KoShape* shape, shapes ) {
        kDebug() << "shape: " << shape->name();
    }
    
    store->disallowNameExpansion();
    KoOdfWriteStore odfStore( store );
    KoXmlWriter* manifestWriter = odfStore.manifestWriter( "application/vnd.oasis.opendocument.graphics" );
    KoEmbeddedDocumentSaver embeddedSaver;
    KoDocument::SavingContext documentContext( odfStore, embeddedSaver );
    
    if( !store->open( "content.xml" ) )
        return false;
    
    KoStoreDevice storeDev( store );
    KoXmlWriter * docWriter = KoOdfWriteStore::createOasisXmlWriter( &storeDev, "office:document-content" );
    
    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter( &masterStyles, 1 );
    
    KoPageLayout page;
    page.format = KoPageFormat::defaultFormat();
    page.orientation = KoPageFormat::Portrait;
    // XXX: this is in pixels -- should be in points?
    page.width = image()->width();
    page.height = image()->height();
    
    KoGenStyles mainStyles;
    KoGenStyle pageLayout = page.saveOasis();
    QString layoutName = mainStyles.lookup( pageLayout, "PL" );
    KoGenStyle masterPage( KoGenStyle::StyleMaster );
    masterPage.addAttribute( "style:page-layout-name", layoutName );
    mainStyles.lookup( masterPage, "Default", KoGenStyles::DontForceNumbering );
    
    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );
    
    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( "office:drawing" );

    KoShapeSavingContext shapeContext( contentTmpWriter, mainStyles, documentContext.embeddedSaver );

    shapeContext.xmlWriter().startElement( "draw:page" );
    shapeContext.xmlWriter().addAttribute( "draw:name", "" );
    shapeContext.xmlWriter().addAttribute( "draw:id", "page1");
    shapeContext.xmlWriter().addAttribute( "draw:master-page-name", "Default");

    saveOdf(shapeContext);

    shapeContext.xmlWriter().endElement(); // draw:page

    contentTmpWriter.endElement(); // office:drawing
    contentTmpWriter.endElement(); // office:body

    mainStyles.saveOdfAutomaticStyles( docWriter, false );

    // And now we can copy over the contents from the tempfile to the real one
    contentTmpFile.seek(0);
    docWriter->addCompleteElement( &contentTmpFile );

    docWriter->endElement(); // Root element
    docWriter->endDocument();
    delete docWriter;

    if( !store->close() )
        return false;

    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if ( ! mainStyles.saveOdfStylesDotXml( store, manifestWriter ) ) {
        return false;
    }
   
   manifestWriter->addManifestEntry("settings.xml", "text/xml");

    if( ! shapeContext.saveImages( store, manifestWriter ) )
        return false;

    // Write out manifest file
    if ( !odfStore.closeManifestWriter() )
    {
        kDebug(41001) << "closing manifestWriter failed";
        return false;
    }

    return true;
}

#include "kis_shape_layer.moc"
