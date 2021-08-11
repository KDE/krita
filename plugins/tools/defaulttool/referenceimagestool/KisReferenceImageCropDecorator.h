/*
 *  SPDX-FileCopyrightText: 2021 Sachin Jindal <jindalsachin01@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
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

    void paint(QPainter &gc, const KoViewConverter &converter);

    void setReferenceImage(KisReferenceImage *reference);

private:
    KisReferenceImage* m_referenceImage;
    QRectF m_cropBorderRect;
};

#endif // KISREFERENCEIMAGECROPDECORATOR_H
