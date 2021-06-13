#include "kisreferenceimagecropdecorator.h"
#include <KoShape.h>
#include <KoViewConverter.h>

#include <QPainter>

KisReferenceImageCropDecorator::KisReferenceImageCropDecorator()
    : mHandleSize(10)
{

}

void KisReferenceImageCropDecorator::setReferenceImage(KisReferenceImage *r)
{
    mReference = r;
}

void KisReferenceImageCropDecorator::paint(QPainter &gc, const KoViewConverter &converter)
{
    if(!mReference) {
        return;
    }

    gc.save();

    QRectF shapeRect = converter.documentToView(mReference->boundingRect());
    QRectF borderRect = converter.documentToView(mCropBorderRect);

    QPainterPath path;

    path.addRect(shapeRect);
    path.addRect(borderRect);
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

QPainterPath KisReferenceImageCropDecorator::handlesPath()
{
    QPainterPath path;

    path.addRect(upperLeftHandleRect());
    path.addRect(upperRightHandleRect());
    path.addRect(lowerLeftHandleRect());
    path.addRect(lowerRightHandleRect());
    path.addRect(upperHandleRect());
    path.addRect(lowerHandleRect());
    path.addRect(leftHandleRect());
    path.addRect(rightHandleRect());

    return path;
}

QRectF KisReferenceImageCropDecorator::lowerRightHandleRect()
{
    return QRectF(mCropBorderRect.right() - mHandleSize / 2.0, mCropBorderRect.bottom() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::upperRightHandleRect()
{
    return QRectF(mCropBorderRect.right() - mHandleSize / 2.0 , mCropBorderRect.top() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::lowerLeftHandleRect()
{
    return QRectF(mCropBorderRect.left() - mHandleSize / 2.0 , mCropBorderRect.bottom() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::upperLeftHandleRect()
{
    return QRectF(mCropBorderRect.left() - mHandleSize / 2.0, mCropBorderRect.top() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::lowerHandleRect()
{
    return QRectF(mCropBorderRect.left() + (mCropBorderRect.width() - mHandleSize) / 2.0 , mCropBorderRect.bottom() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::rightHandleRect()
{
    return QRectF(mCropBorderRect.right() - mHandleSize / 2.0 , mCropBorderRect.top() + (mCropBorderRect.height() - mHandleSize) / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::upperHandleRect()
{
    return QRectF(mCropBorderRect.left() + (mCropBorderRect.width() - mHandleSize) / 2.0 , mCropBorderRect.top() - mHandleSize / 2.0, mHandleSize, mHandleSize);
}

QRectF KisReferenceImageCropDecorator::leftHandleRect()
{
    return QRectF(mCropBorderRect.left() - mHandleSize / 2.0, mCropBorderRect.top() + (mCropBorderRect.height() - mHandleSize) / 2.0, mHandleSize, mHandleSize);
}
