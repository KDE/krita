#ifndef KISREFERENCEIMAGECROPDECORATOR_H
#define KISREFERENCEIMAGECROPDECORATOR_H

#include "KisReferenceImage.h"

class KisReferenceImage;

class KisReferenceImageCropDecorator
{
public:
    KisReferenceImageCropDecorator();
    ~KisReferenceImageCropDecorator() {};

    void paint(QPainter &gc, const KoViewConverter &converter);

    void setReferenceImage(KisReferenceImage *reference);

    void setBorderRect(QRectF);

    QPainterPath handlesPath();
    QRectF lowerRightHandleRect();
    QRectF upperRightHandleRect();
    QRectF lowerLeftHandleRect();
    QRectF upperLeftHandleRect();
    QRectF lowerHandleRect();
    QRectF rightHandleRect();
    QRectF upperHandleRect();
    QRectF leftHandleRect();

private:
    KisReferenceImage* mReference;
    int mHandleSize;
    QRectF mCropBorderRect;

};

#endif // KISREFERENCEIMAGECROPDECORATOR_H
