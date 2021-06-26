#include "kisreferenceimagecropdecorator.h"
#include <KoShape.h>
#include <KoViewConverter.h>
#include "kis_canvas2.h"

#include <QPainter>

KisReferenceImageCropDecorator::KisReferenceImageCropDecorator()
    : mHandleSize(10)
{

}

void KisReferenceImageCropDecorator::setReferenceImage(KisReferenceImage *r)
{
    mReference = r;
    mCropBorderRect = r->getCropRect();
}

void KisReferenceImageCropDecorator::paint(QPainter &gc, const KoViewConverter &converter, KoCanvasBase *canvas)
{
    if(!mReference) {
        return;
    }

    auto kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    const KisCoordinatesConverter *conv = kisCanvas->coordinatesConverter();

    gc.save();

    QRectF shapeRect = converter.documentToView(mReference->boundingRect());
    QRectF borderRect = conv->imageToDocument(mCropBorderRect);
    mCropBorderRect = converter.documentToView(borderRect);

    QPainterPath path;

    path.addRect(shapeRect);
    path.addRect(mCropBorderRect);
    gc.setPen(Qt::NoPen);
    gc.setBrush(QColor(0, 0, 0, 200));
    gc.drawPath(path);

    // Handles
    QPen pen(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(Qt::black);
    gc.setPen(pen);
    gc.setBrush(QColor(200, 200, 200, 200));
    gc.drawPath(handlesPath());

    gc.restore();

}
