#ifndef KISREFERENCEIMAGECROPDECORATOR_H
#define KISREFERENCEIMAGECROPDECORATOR_H

#include "KisReferenceImage.h"
#include "KoCanvasBase.h"

class KisReferenceImage;

class KisReferenceImageCropDecorator
{
public:
    KisReferenceImageCropDecorator();
    ~KisReferenceImageCropDecorator() {};

    void paint(QPainter &gc, const KoViewConverter &converter, KoCanvasBase *canvas);

    void setReferenceImage(KisReferenceImage *reference);

    void setBorderRect(QRectF);

    QPainterPath handlesPath();

private:
    KisReferenceImage* mReference;
    int mHandleSize;
    QRectF mCropBorderRect;

};

#endif // KISREFERENCEIMAGECROPDECORATOR_H
