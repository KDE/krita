/*
 *  SPDX-FileCopyrightText: 2021 Sachin Jindal <jindalsachin01@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISREFERENCEIMAGESELECTIONDECORATOR_H
#define KISREFERENCEIMAGESELECTIONDECORATOR_H

#include "KisReferenceImage.h"
#include "KoCanvasBase.h"
#include "KoSelection.h"

class KisReferenceImage;

class KisReferenceImageSelectionDecorator
{
public:
    KisReferenceImageSelectionDecorator();
    ~KisReferenceImageSelectionDecorator() {};

    void paint(QPainter &gc);

    void setSelection(KoSelection *selection);
    void drawHandle(QPainter &gc, KoShape *shape, QRectF handleArea, bool paintHotPosition = false);

private:
    KisReferenceImage* m_referenceImage;
    QRectF m_cropBorderRect;
    KoSelection *m_selection;
    int m_handleRadius;
    KoFlake::AnchorPosition m_hotPosition;
};

#endif // KISREFERENCEIMAGESELECTIONDECORATOR_H
