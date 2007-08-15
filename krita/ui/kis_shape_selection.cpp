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

#include "kis_painter.h"
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
    return 0;
}

#include "kis_shape_selection.moc"
