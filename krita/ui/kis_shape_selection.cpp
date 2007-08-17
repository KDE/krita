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

#include <KoLineBorder.h>
#include <KoPathShape.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceRegistry.h>
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_shape_selection_model.h"

#include <kdebug.h>

KisShapeSelection::KisShapeSelection(KisImageSP image, KisPaintDeviceSP dev)
    : KoShapeContainer(new KisShapeSelectionModel(image, dev, this))
    , m_image(image)
    , m_parentPaintDevice(dev)
{
    m_dashOffset = 0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(repaintTimerEvent()));
    m_timer->start(300);
    setShapeId("KisShapeSelection");
    setSelectable(false);
    m_dirty = false;
}

KisShapeSelection::~KisShapeSelection()
{
}

bool KisShapeSelection::loadOdf(const KoXmlElement&, KoShapeLoadingContext&)
{
    return false;
}

void KisShapeSelection::saveOdf(KoShapeSavingContext&) const
{
}

void KisShapeSelection::repaintTimerEvent()
{
    m_dashOffset++;
    if(m_dashOffset>7) m_dashOffset = 0;
    repaint();
}

QPainterPath KisShapeSelection::selectionOutline()
{
    if(m_dirty) {
        QList<KoShape*> shapesList = iterator();

        QPainterPath outline;
        KoPathShape* pathShape;
        foreach( KoShape * shape, shapesList )
        {
            pathShape = dynamic_cast<KoPathShape*>( shape );
            if(pathShape) {
                QMatrix shapeMatrix = shape->absoluteTransformation(0);

            outline = outline.united(shapeMatrix.map(shape->outline()));
            }
        }
        m_outline = outline;
        m_dirty = false;
    }
    return m_outline;
}

void KisShapeSelection::paintComponent(QPainter& painter, const KoViewConverter& converter)
{
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);

    QVector<qreal> dashes;
    qreal space = 4;
    dashes << 4 << space;

    QPainterPathStroker stroker;
    stroker.setWidth(0);
    stroker.setDashPattern(dashes);
    stroker.setDashOffset(m_dashOffset-4);

    painter.setRenderHint(QPainter::Antialiasing);
    QColor outlineColor = Qt::black;

    QMatrix zoomMatrix;
    zoomMatrix.scale(zoomX, zoomY);

    QPainterPath stroke = stroker.createStroke(zoomMatrix.map(selectionOutline()));
    painter.fillPath(stroke, outlineColor);
}

void KisShapeSelection::renderToProjection(KisSelection* projection)
{
    KisPainter painter(projection);
    painter.setPaintColor(KoColor(Qt::black, projection->colorSpace()));
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.setOpacity(OPACITY_OPAQUE);
    painter.setCompositeOp(projection->colorSpace()->compositeOp(COMPOSITE_OVER));

    QMatrix resolutionMatrix;
    resolutionMatrix.scale(m_image->xRes(), m_image->yRes());
    painter.fillPainterPath(resolutionMatrix.map(selectionOutline()));
}

void KisShapeSelection::renderToProjection(KisSelection* projection, const QRect& r)
{
    QMatrix resolutionMatrix;
    resolutionMatrix.scale(m_image->xRes(), m_image->yRes());

    QTime t;
    t.start();

    KisPaintDeviceSP tmpMask = new KisPaintDevice(KoColorSpaceRegistry::instance()->colorSpace("GRAY", 0), "tmp");

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
    painter.bitBlt(r.x(), r.y(), COMPOSITE_OVER, tmpMask, r.x(), r.y(), r.width(), r.height());
    painter.end();
    kDebug(41010) << "Shape selection rendering: " << t.elapsed();
}

void KisShapeSelection::setDirty()
{
    m_dirty = true;
}


KisShapeSelectionFactory::KisShapeSelectionFactory( QObject* parent)
    : KoShapeFactory( parent, "KisShapeSelection", "selection shape container" )
{
}

KoShape* KisShapeSelectionFactory::createDefaultShape() const
{
    return 0;
}

KoShape* KisShapeSelectionFactory::createShape( const KoProperties* params ) const
{
    Q_UNUSED( params );
    return 0;
}

#include "kis_shape_selection.moc"
