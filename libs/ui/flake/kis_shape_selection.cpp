/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2011 Jan Hambrecht <jaham@gmx.net>
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
#include <kundo2command.h>
#include <QMimeData>

#include <QTemporaryFile>

#include <KoShapeStroke.h>
#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoCompositeOp.h>
#include <KoShapeManager.h>
#include <KisDocument.h>

#include <KoEmbeddedDocumentSaver.h>
#include <KoGenStyles.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoOdfWriteStore.h>
#include <KoXmlNS.h>
#include <KoShapeRegistry.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoShapeController.h>
#include <KoShapeSavingContext.h>
#include <KoStoreDevice.h>
#include <KoShapeTransformCommand.h>
#include <KoElementReference.h>

#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_selection.h>

#include "kis_shape_selection_model.h"
#include "kis_shape_selection_canvas.h"
#include "kis_take_all_shapes_command.h"
#include "kis_image_view_converter.h"
#include "kis_shape_layer.h"


#include <kis_debug.h>

KisShapeSelection::KisShapeSelection(KoShapeControllerBase *shapeControllerBase, KisImageWSP image, KisSelectionWSP selection)
    : KoShapeLayer(m_model = new KisShapeSelectionModel(image, selection, this))
    , m_image(image)
    , m_shapeControllerBase(shapeControllerBase)
{
    Q_ASSERT(m_image);
    setShapeId("KisShapeSelection");
    setSelectable(false);
    m_converter = new KisImageViewConverter(image);
    m_canvas = new KisShapeSelectionCanvas(shapeControllerBase);
    m_canvas->shapeManager()->addShape(this);

    m_model->setObjectName("KisShapeSelectionModel");
    m_model->moveToThread(image->thread());
    m_canvas->setObjectName("KisShapeSelectionCanvas");
    m_canvas->moveToThread(image->thread());
}

KisShapeSelection::~KisShapeSelection()
{
    m_model->setShapeSelection(0);
    delete m_canvas;
    delete m_converter;
}

KisShapeSelection::KisShapeSelection(const KisShapeSelection& rhs, KisSelection* selection)
    : KoShapeLayer(m_model = new KisShapeSelectionModel(rhs.m_image, selection, this))
{
    m_image = rhs.m_image;
    m_shapeControllerBase = rhs.m_shapeControllerBase;
    m_converter = new KisImageViewConverter(m_image);
    m_canvas = new KisShapeSelectionCanvas(m_shapeControllerBase);
    m_canvas->shapeManager()->addShape(this);

    Q_FOREACH (KoShape *shape, rhs.shapes()) {
        KoShape *clonedShape = shape->cloneShape();
        KIS_SAFE_ASSERT_RECOVER(clonedShape) { continue; }
        this->addShape(clonedShape);
    }
}

KisSelectionComponent* KisShapeSelection::clone(KisSelection* selection)
{
    return new KisShapeSelection(*this, selection);
}

bool KisShapeSelection::saveSelection(KoStore * store) const
{
    const QSizeF sizeInPx = m_image->bounds().size();
    const QSizeF sizeInPt(sizeInPx.width() / m_image->xRes(), sizeInPx.height() / m_image->yRes());

    return KisShapeLayer::saveShapesToStore(store, this->shapes(), sizeInPt);
}

bool KisShapeSelection::loadSelection(KoStore* store)
{
    QSizeF fragmentSize; // unused!

    // FIXME: we handle xRes() only!
    KIS_SAFE_ASSERT_RECOVER_NOOP(qFuzzyCompare(m_image->xRes(), m_image->yRes()));
    const qreal resolutionPPI = 72.0 * m_image->xRes();

    QList<KoShape*> shapes;

    if (store->open("content.svg")) {
        KoStoreDevice storeDev(store);
        storeDev.open(QIODevice::ReadOnly);

        shapes = KisShapeLayer::createShapesFromSvg(&storeDev,
                                                    "", m_image->bounds(),
                                                    resolutionPPI, m_canvas->shapeController()->resourceManager(),
                                                    &fragmentSize);

        store->close();

        Q_FOREACH (KoShape *shape, shapes) {
            addShape(shape);
        }

        return true;
    }


    KoOdfReadStore odfStore(store);
    QString errorMessage;

    odfStore.loadAndParse(errorMessage);

    if (!errorMessage.isEmpty()) {
        dbgKrita << errorMessage;
        return false;
    }

    KoXmlElement contents = odfStore.contentDoc().documentElement();

    //    dbgKrita <<"Start loading OASIS document..." << contents.text();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().localName();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().namespaceURI();
    //    dbgKrita <<"Start loading OASIS contents..." << contents.lastChild().isElement();

    KoXmlElement body(KoXml::namedItemNS(contents, KoXmlNS::office, "body"));

    if (body.isNull()) {
        dbgKrita << "No office:body found!";
        //setErrorMessage( i18n( "Invalid OASIS document. No office:body tag found." ) );
        return false;
    }

    body = KoXml::namedItemNS(body, KoXmlNS::office, "drawing");
    if (body.isNull()) {
        dbgKrita << "No office:drawing found!";
        //setErrorMessage( i18n( "Invalid OASIS document. No office:drawing tag found." ) );
        return false;
    }

    KoXmlElement page(KoXml::namedItemNS(body, KoXmlNS::draw, "page"));
    if (page.isNull()) {
        dbgKrita << "No office:drawing found!";
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
        dbgKrita << "No master page found!";
        return false;
    }

    KoOdfLoadingContext context(odfStore.styles(), odfStore.store());
    KoShapeLoadingContext shapeContext(context, 0);

    KoXmlElement layerElement;
    forEachElement(layerElement, context.stylesReader().layerSet()) {
        if (!loadOdf(layerElement, shapeContext)) {
            dbgKrita << "Could not load vector layer!";
            return false;
        }
    }

    KoXmlElement child;
    forEachElement(child, page) {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, shapeContext);
        if (shape) {
            addShape(shape);
        }
    }

    return true;
}

void KisShapeSelection::setUpdatesEnabled(bool enabled)
{
    m_model->setUpdatesEnabled(enabled);
}

bool KisShapeSelection::updatesEnabled() const
{
    return m_model->updatesEnabled();
}

KUndo2Command* KisShapeSelection::resetToEmpty()
{
    return new KisTakeAllShapesCommand(this, true);
}

bool KisShapeSelection::isEmpty() const
{
    return !m_model->count();
}

QPainterPath KisShapeSelection::outlineCache() const
{
    return m_outline;
}

bool KisShapeSelection::outlineCacheValid() const
{
    return true;
}

void KisShapeSelection::recalculateOutlineCache()
{
    QList<KoShape*> shapesList = shapes();

    QPainterPath outline;
    Q_FOREACH (KoShape * shape, shapesList) {
        QTransform shapeMatrix = shape->absoluteTransformation(0);
        outline = outline.united(shapeMatrix.map(shape->outline()));
    }

    QTransform resolutionMatrix;
    resolutionMatrix.scale(m_image->xRes(), m_image->yRes());

    m_outline = resolutionMatrix.map(outline);
}

void KisShapeSelection::paintComponent(QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisShapeSelection::renderToProjection(KisPaintDeviceSP projection)
{
    Q_ASSERT(projection);
    Q_ASSERT(m_image);

    QRectF boundingRect = outlineCache().boundingRect();
    renderSelection(projection, boundingRect.toAlignedRect());
}

void KisShapeSelection::renderToProjection(KisPaintDeviceSP projection, const QRect& r)
{
    Q_ASSERT(projection);
    renderSelection(projection, r);
}

void KisShapeSelection::renderSelection(KisPaintDeviceSP projection, const QRect& r)
{
    Q_ASSERT(projection);
    Q_ASSERT(m_image);

    const qint32 MASK_IMAGE_WIDTH = 256;
    const qint32 MASK_IMAGE_HEIGHT = 256;

    QImage polygonMaskImage(MASK_IMAGE_WIDTH, MASK_IMAGE_HEIGHT, QImage::Format_ARGB32);
    QPainter maskPainter(&polygonMaskImage);
    maskPainter.setRenderHint(QPainter::Antialiasing, true);

    // Break the mask up into chunks so we don't have to allocate a potentially very large QImage.
    for (qint32 x = r.x(); x < r.x() + r.width(); x += MASK_IMAGE_WIDTH) {
        for (qint32 y = r.y(); y < r.y() + r.height(); y += MASK_IMAGE_HEIGHT) {

            maskPainter.fillRect(polygonMaskImage.rect(), Qt::black);
            maskPainter.translate(-x, -y);
            maskPainter.fillPath(outlineCache(), Qt::white);
            maskPainter.translate(x, y);

            qint32 rectWidth = qMin(r.x() + r.width() - x, MASK_IMAGE_WIDTH);
            qint32 rectHeight = qMin(r.y() + r.height() - y, MASK_IMAGE_HEIGHT);

            KisSequentialIterator it(projection, QRect(x, y, rectWidth, rectHeight));
            while (it.nextPixel()) {
                (*it.rawData()) = qRed(polygonMaskImage.pixel(it.x() - x, it.y() - y));
            }
        }
    }
}

KoShapeManager* KisShapeSelection::shapeManager() const
{
    return m_canvas->shapeManager();
}

KisShapeSelectionFactory::KisShapeSelectionFactory()
    : KoShapeFactoryBase("KisShapeSelection", "selection shape container")
{
    setHidden(true);
}

void KisShapeSelection::moveX(qint32 x)
{
    Q_FOREACH (KoShape* shape, shapeManager()->shapes()) {
        if (shape != this) {
            QPointF pos = shape->position();
            shape->setPosition(QPointF(pos.x() + x/m_image->xRes(), pos.y()));
        }
    }
}

void KisShapeSelection::moveY(qint32 y)
{
    Q_FOREACH (KoShape* shape, shapeManager()->shapes()) {
        if (shape != this) {
            QPointF pos = shape->position();
            shape->setPosition(QPointF(pos.x(), pos.y() + y/m_image->yRes()));
        }
    }
}

// TODO same code as in vector layer, refactor!
KUndo2Command* KisShapeSelection::transform(const QTransform &transform) {
    QList<KoShape*> shapes = m_canvas->shapeManager()->shapes();
    if(shapes.isEmpty()) return 0;

    QTransform realTransform = m_converter->documentToView() *
            transform * m_converter->viewToDocument();

    QList<QTransform> oldTransformations;
    QList<QTransform> newTransformations;

    // this code won't work if there are shapes, that inherit the transformation from the parent container.
    // the chart and tree shapes are examples for that, but they aren't used in krita and there are no other shapes like that.
    Q_FOREACH (const KoShape* shape, shapes) {
        QTransform oldTransform = shape->transformation();
        oldTransformations.append(oldTransform);
        if (dynamic_cast<const KoShapeGroup*>(shape)) {
            newTransformations.append(oldTransform);
        } else {
            QTransform globalTransform = shape->absoluteTransformation(0);
            QTransform localTransform = globalTransform * realTransform * globalTransform.inverted();
            newTransformations.append(localTransform*oldTransform);
        }
    }

    return new KoShapeTransformCommand(shapes, oldTransformations, newTransformations);
}
