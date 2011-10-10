/*
    Part of Calligra Suite - Map Shape
    Copyright (C) 2007 Thomas Zander <zander@kde.org>
    Copyright (C) 2008-2009 Simon Schmeißer <mail_to_wrt@gmx.de>
    Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "MapShape.h"
#include <MarbleWidget.h>
#include <MarbleModel.h>
#include <KoImageData.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <kdebug.h>
#include <KoViewConverter.h>
#include <QPainter>
#include <KoImageCollection.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>

class MapShapePrivate {
public:
    MapShapePrivate() {
        m_marbleWidget = new Marble::MarbleWidget();
        m_marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
        m_marbleWidget->setProjection(Marble::Equirectangular);
        m_marbleWidget->centerOn(20.8134,52.16635);
        m_marbleWidget->zoomView(1200);
        m_marbleWidget->setShowOverviewMap(false);
        m_screenSize = QSize(0, 0);
    }
    ~MapShapePrivate(){
        delete m_marbleWidget;
    }
    Marble::MarbleWidget* m_marbleWidget;
    //KoImageData* m_image;
    QSize m_screenSize;
    QImage m_cacheImage;
    KoImageCollection *m_imageCollection;
};

MapShape::MapShape()
    : KoFrameShape(KoXmlNS::calligra, "map"),
    d(new MapShapePrivate())
{
    connect(d->m_marbleWidget->model(), SIGNAL(modelChanged()), this, SIGNAL(requestUpdate()));
}
MapShape::~MapShape()
{
    delete d;
}

void MapShape::requestUpdate()
{
    update();
}

void MapShape::paint(QPainter& painter, const KoViewConverter& converter)
{
    QRectF target = converter.documentToView(QRectF(QPointF(0,0), size()));
    QSize newSize = target.size().toSize();
    
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    // FIXME: remove this arbitary rule for determining if it's a preview or a full-size flake
    if ((newSize != d->m_screenSize) && (zoomX > 0.5)){
        d->m_screenSize = newSize;
        d->m_marbleWidget->setFixedSize(d->m_screenSize.width(), d->m_screenSize.height());
        kDebug() << "paint: size of the image: " << d->m_screenSize ;
    }

    d->m_cacheImage = QImage(d->m_screenSize, QImage::Format_ARGB32);
    d->m_cacheImage.fill(Qt::transparent);
    d->m_marbleWidget->render(&d->m_cacheImage);
    
    //painter.drawPixmap(target.toRect(), d->m_cacheImage);
    painter.drawImage(target.toRect(), d->m_cacheImage);
}

void MapShape::saveOdf(KoShapeSavingContext& context) const
{
    kDebug() << "saveOdf: size of the image " << d->m_screenSize ;
    KoImageData *data = d->m_imageCollection->createImageData(d->m_cacheImage);

    QString pictureName = context.imageHref(data);
    
    KoXmlWriter &writer = context.xmlWriter();
    
    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    // first comes the real map object, it should be loaded if possible
    // OpenOffice (2.x?) will ignore and remove this
    writer.startElement("calligra:map");
    writer.addAttribute("mapThemeId", d->m_marbleWidget->mapThemeId());
    switch(d->m_marbleWidget->projection()){
        case Marble::Spherical:
            writer.addAttribute("projection", "Spherical");
            break;
        case Marble::Equirectangular:
            writer.addAttribute("projection", "Equirectangular");
            break;
        case Marble::Mercator:
            writer.addAttribute("projection", "Mercator");
            break;
    }
    writer.addAttribute("zoom", d->m_marbleWidget->zoom());
    writer.addAttribute("centerLongitude", d->m_marbleWidget->centerLongitude());
    writer.addAttribute("centerLatitude", d->m_marbleWidget->centerLatitude());
    writer.endElement();
    
    // as a fallback add an image
    writer.startElement("draw:image");
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", pictureName);
    writer.endElement(); // draw:image
    
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw-frame
    
    context.addDataCenter(d->m_imageCollection);
}

bool MapShape::loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}


bool MapShape::loadOdfFrameElement(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    Q_UNUSED(context)
    const QString mapThemeId = element.attribute("mapThemeId");
    if (!mapThemeId.isEmpty())
        d->m_marbleWidget->setMapThemeId(mapThemeId);

    const QString projection = element.attribute("projection");
    if (!projection.isEmpty()){
        if(projection == "Spherical")
            d->m_marbleWidget->setProjection(Marble::Spherical);
        else if(projection == "Equirectangular")
            d->m_marbleWidget->setProjection(Marble::Equirectangular);
        else if(projection == "Mercator")
            d->m_marbleWidget->setProjection(Marble::Mercator);
    }

    const QString zoom = element.attribute("zoom");

    if(!zoom.isEmpty())
        d->m_marbleWidget->zoomView(zoom.toInt());

    const QString centerLatitude = element.attribute("centerLatitude");
    if (!centerLatitude.isEmpty())
        d->m_marbleWidget->setCenterLatitude(centerLatitude.toDouble());

    const QString centerLongitude = element.attribute("centerLongitude");

    if (!centerLongitude.isEmpty())
        d->m_marbleWidget->setCenterLongitude(centerLongitude.toDouble());

    kDebug() << "themId    :" << mapThemeId;
    kDebug() << "projecton :" << projection;
    kDebug() << "zoom      :" << zoom;
    kDebug() << "latitude  :" << centerLatitude;
    kDebug() << "Longitude :" << centerLongitude;
    return true;
}

KoImageCollection* MapShape::imageCollection() const
{
    return d->m_imageCollection;
}

void MapShape::setImageCollection(KoImageCollection* collection)
{
    d->m_imageCollection = collection;
}

Marble::MarbleWidget* MapShape::marbleWidget() const
{
    return d->m_marbleWidget;
}
