/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_shape_selection.h"


#include <QPainter>
#include <QTimer>

#include <ktemporaryfile.h>

#include <KoLineBorder.h>
#include <KoPathShape.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeManager.h>
#include <KoDocument.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoGenStyle.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoOdfWriteStore.h>
#include <KoXmlNS.h>
#include <KoShapeRegistry.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoShapeSavingContext.h>
#include <KoStoreDevice.h>


#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_shape_selection_model.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_shape_selection_canvas.h"

#include <kis_debug.h>

KisShapeSelection::KisShapeSelection(KisImageWSP image, KisSelectionSP selection)
        : KoShapeLayer(new KisShapeSelectionModel(image, selection, this))
        , m_image(image)
{
    Q_ASSERT(m_image);
    setShapeId("KisShapeSelection");
    setSelectable(false);
    m_dirty = false;
    m_canvas = new KisShapeSelectionCanvas();
    m_canvas->shapeManager()->add(this);

}

KisShapeSelection::~KisShapeSelection()
{
    delete m_canvas;
}

KisShapeSelection::KisShapeSelection(const KisShapeSelection& rhs)
        : KoShapeLayer(rhs)
{
    m_dirty = rhs.m_dirty;
    m_image = rhs.m_image;
    m_canvas = new KisShapeSelectionCanvas();
    m_canvas->shapeManager()->add(this);
}

KisSelectionComponent* KisShapeSelection::clone()
{
    return new KisShapeSelection(*this);
}

bool KisShapeSelection::saveSelection(KoStore * store) const
{
    store->disallowNameExpansion();
    KoOdfWriteStore odfStore(store);
    KoXmlWriter* manifestWriter = odfStore.manifestWriter("application/vnd.oasis.opendocument.graphics");
    KoEmbeddedDocumentSaver embeddedSaver;
    KoDocument::SavingContext documentContext(odfStore, embeddedSaver);

    if (!store->open("content.xml"))
        return false;

    KoStoreDevice storeDev(store);
    KoXmlWriter * docWriter = KoOdfWriteStore::createOasisXmlWriter(&storeDev, "office:document-content");

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter(&masterStyles, 1);

    KoPageLayout page;
    page.format = KoPageFormat::defaultFormat();
    QRectF rc = boundingRect();
    page.width = rc.width();
    page.height = rc.height();
    if (page.width > page.height) {
        page.orientation = KoPageFormat::Landscape;
    } else {
        page.orientation = KoPageFormat::Portrait;
    }

    KoGenStyles mainStyles;
    KoGenStyle pageLayout = page.saveOdf();
    QString layoutName = mainStyles.lookup(pageLayout, "PL");
    KoGenStyle masterPage(KoGenStyle::StyleMaster);
    masterPage.addAttribute("style:page-layout-name", layoutName);
    mainStyles.lookup(masterPage, "Default", KoGenStyles::DontForceNumbering);

    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter(&contentTmpFile, 1);

    contentTmpWriter.startElement("office:body");
    contentTmpWriter.startElement("office:drawing");

    KoShapeSavingContext shapeContext(contentTmpWriter, mainStyles, documentContext.embeddedSaver);

    shapeContext.xmlWriter().startElement("draw:page");
    shapeContext.xmlWriter().addAttribute("draw:name", "");
    shapeContext.xmlWriter().addAttribute("draw:id", "page1");
    shapeContext.xmlWriter().addAttribute("draw:master-page-name", "Default");

    saveOdf(shapeContext);

    shapeContext.xmlWriter().endElement(); // draw:page

    contentTmpWriter.endElement(); // office:drawing
    contentTmpWriter.endElement(); // office:body

    mainStyles.saveOdfAutomaticStyles(docWriter, false);

    // And now we can copy over the contents from the tempfile to the real one
    contentTmpFile.seek(0);
    docWriter->addCompleteElement(&contentTmpFile);

    docWriter->endElement(); // Root element
    docWriter->endDocument();
    delete docWriter;

    if (!store->close())
        return false;

    manifestWriter->addManifestEntry("content.xml", "text/xml");

    if (! mainStyles.saveOdfStylesDotXml(store, manifestWriter)) {
        return false;
    }

    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    if (! shapeContext.saveDataCenter(documentContext.odfStore.store(), documentContext.odfStore.manifestWriter()))
        return false;

    // Write out manifest file
    if (!odfStore.closeManifestWriter()) {
        dbgImage << "closing manifestWriter failed";
        return false;
    }

    return true;
}

bool KisShapeSelection::loadSelection(KoStore* store)
{
    KoOdfReadStore odfStore(store);
    QString errorMessage;

    odfStore.loadAndParse(errorMessage);

    if (!errorMessage.isEmpty()) {
        qDebug() << errorMessage;
        return false;
    }

    KoXmlElement contents = odfStore.contentDoc().documentElement();

//    qDebug() <<"Start loading OASIS document..." << contents.text();
//    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().localName();
//    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().namespaceURI();
//    qDebug() <<"Start loading OASIS contents..." << contents.lastChild().isElement();

    KoXmlElement body(KoXml::namedItemNS(contents, KoXmlNS::office, "body"));

    if (body.isNull()) {
        qDebug() << "No office:body found!";
        //setErrorMessage( i18n( "Invalid OASIS document. No office:body tag found." ) );
        return false;
    }

    body = KoXml::namedItemNS(body, KoXmlNS::office, "drawing");
    if (body.isNull()) {
        qDebug() << "No office:drawing found!";
        //setErrorMessage( i18n( "Invalid OASIS document. No office:drawing tag found." ) );
        return false;
    }

    KoXmlElement page(KoXml::namedItemNS(body, KoXmlNS::draw, "page"));
    if (page.isNull()) {
        qDebug() << "No office:drawing found!";
        //setErrorMessage( i18n( "Invalid OASIS document. No draw:page tag found." ) );
        return false;
    }

    KoXmlElement * master = 0;
    if (odfStore.styles().masterPages().contains("Standard"))
        master = odfStore.styles().masterPages().value("Standard");
    else if (odfStore.styles().masterPages().contains("Default"))
        master = odfStore.styles().masterPages().value("Default");
    else if (! odfStore.styles().masterPages().empty())
        master = odfStore.styles().masterPages().begin().value();

    if (master) {
        const KoXmlElement *style = odfStore.styles().findStyle(
                                        master->attributeNS(KoXmlNS::style, "page-layout-name", QString()));
        KoPageLayout pageLayout;
        pageLayout.loadOdf(*style);
        setSize(QSizeF(pageLayout.width, pageLayout.height));
    } else {
        kWarning() << "No master page found!";
        return false;
    }

    KoOdfLoadingContext context(odfStore.styles(), odfStore.store());
    KoShapeLoadingContext shapeContext(context, 0);

    KoXmlElement layerElement;
    forEachElement(layerElement, context.stylesReader().layerSet()) {
        if (!loadOdf(layerElement, shapeContext)) {
            kWarning() << "Could not load shape layer!";
            return false;
        }
    }

    KoXmlElement child;
    forEachElement(child, page) {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, shapeContext);
        if (shape) {
            addChild(shape);
        }
    }

    return true;

}


void KisShapeSelection::addChild(KoShape *object)
{
    KoShapeLayer::addChild(object);
    m_canvas->shapeManager()->add(object);
}

void KisShapeSelection::removeChild(KoShape *object)
{
    m_canvas->shapeManager()->remove(object);
    KoShapeLayer::removeChild(object);
}

QPainterPath KisShapeSelection::selectionOutline()
{
    if (m_dirty) {
        QList<KoShape*> shapesList = childShapes();

        QPainterPath outline;
        foreach(KoShape * shape, shapesList) {
            QMatrix shapeMatrix = shape->absoluteTransformation(0);
            outline = outline.united(shapeMatrix.map(shape->outline()));
        }
        m_outline = outline;
        m_dirty = false;
    }
    return m_outline;
}

void KisShapeSelection::paintComponent(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisShapeSelection::renderToProjection(KisSelection* projection)
{
    Q_ASSERT(projection);
    Q_ASSERT(m_image);
    QMatrix resolutionMatrix;
    resolutionMatrix.scale(m_image->xRes(), m_image->yRes());

    QRectF boundingRect = resolutionMatrix.mapRect(selectionOutline().boundingRect());
    renderSelection(projection, boundingRect.toAlignedRect());
}

void KisShapeSelection::renderToProjection(KisSelection* projection, const QRect& r)
{
    Q_ASSERT(projection);
    renderSelection(projection, r);
}

void KisShapeSelection::renderSelection(KisSelection* projection, const QRect& r)
{
    Q_ASSERT(projection);
    Q_ASSERT(m_image);

    QMatrix resolutionMatrix;
    resolutionMatrix.scale(m_image->xRes(), m_image->yRes());

    QTime t;
    t.start();

    KisPaintDeviceSP tmpMask = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    const qint32 MASK_IMAGE_WIDTH = 256;
    const qint32 MASK_IMAGE_HEIGHT = 256;

    QImage polygonMaskImage(MASK_IMAGE_WIDTH, MASK_IMAGE_HEIGHT, QImage::Format_ARGB32);
    QPainter maskPainter(&polygonMaskImage);
    maskPainter.setRenderHint(QPainter::Antialiasing, true);

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.

    for (qint32 x = r.x(); x < r.x() + r.width(); x += MASK_IMAGE_WIDTH) {
        for (qint32 y = r.y(); y < r.y() + r.height(); y += MASK_IMAGE_HEIGHT) {

            maskPainter.fillRect(polygonMaskImage.rect(), QColor(OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, OPACITY_TRANSPARENT, 255));
            maskPainter.translate(-x, -y);
            maskPainter.fillPath(resolutionMatrix.map(selectionOutline()), QColor(OPACITY_OPAQUE, OPACITY_OPAQUE, OPACITY_OPAQUE, 255));
            maskPainter.translate(x, y);

            qint32 rectWidth = qMin(r.x() + r.width() - x, MASK_IMAGE_WIDTH);
            qint32 rectHeight = qMin(r.y() + r.height() - y, MASK_IMAGE_HEIGHT);

            KisRectIterator rectIt = tmpMask->createRectIterator(x, y, rectWidth, rectHeight);

            while (!rectIt.isDone()) {
                (*rectIt.rawData()) = qRed(polygonMaskImage.pixel(rectIt.x() - x, rectIt.y() - y));
                ++rectIt;
            }
        }
    }
    KisPainter painter(projection);
    painter.bitBlt(r.x(), r.y(), tmpMask, r.x(), r.y(), r.width(), r.height());
    painter.end();
    dbgRender << "Shape selection rendering: " << t.elapsed();
}

void KisShapeSelection::setDirty()
{
    m_dirty = true;
}

KoShapeManager* KisShapeSelection::shapeManager() const
{
    return m_canvas->shapeManager();
}

KisShapeSelectionFactory::KisShapeSelectionFactory(QObject* parent)
        : KoShapeFactoryBase(parent, "KisShapeSelection", "selection shape container")
{
    setHidden(true);
}

#include "kis_shape_selection.moc"
