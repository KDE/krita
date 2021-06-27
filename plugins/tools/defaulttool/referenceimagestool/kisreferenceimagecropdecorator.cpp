#include <QPainter>

#include "kisreferenceimagecropdecorator.h"
#include <KoShape.h>
#include <KoViewConverter.h>
#include "kis_canvas2.h"
#include "KisHandlePainterHelper.h"

KisReferenceImageCropDecorator::KisReferenceImageCropDecorator()
{
}

void KisReferenceImageCropDecorator::setReferenceImage(KisReferenceImage *referenceImage)
{
    m_referenceImage = referenceImage;
    m_cropBorderRect = referenceImage->cropRect();
}

void KisReferenceImageCropDecorator::paint(QPainter &gc, const KoViewConverter &converter, KoCanvasBase *canvas)
{
    if(!m_referenceImage) {
        return;
    }

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    const KisCoordinatesConverter *coordinateConverter = kisCanvas->coordinatesConverter();

    gc.save();

    QRectF shapeRect = converter.documentToView(m_referenceImage->boundingRect());
    QRectF borderRect = coordinateConverter->imageToDocument(m_cropBorderRect);
    m_cropBorderRect = converter.documentToView(borderRect);

    QPainterPath path;

    path.addRect(shapeRect);
    path.addRect(m_cropBorderRect);
    gc.setPen(Qt::NoPen);
    gc.setBrush(QColor(0, 0, 0, 200));
    gc.drawPath(path);

    KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(&gc, m_referenceImage, converter, 5);
    helper.setHandleStyle(KisHandleStyle::primarySelection());

    QPolygonF outline = m_referenceImage->cropRect();

    {
        helper.drawHandleRect(outline.value(0));
        helper.drawHandleRect(outline.value(1));
        helper.drawHandleRect(outline.value(2));
        helper.drawHandleRect(outline.value(3));
        helper.drawHandleRect(0.5 * (outline.value(0) + outline.value(1)));
        helper.drawHandleRect(0.5 * (outline.value(1) + outline.value(2)));
        helper.drawHandleRect(0.5 * (outline.value(2) + outline.value(3)));
        helper.drawHandleRect(0.5 * (outline.value(3) + outline.value(0)));
    }
    gc.restore();

}
