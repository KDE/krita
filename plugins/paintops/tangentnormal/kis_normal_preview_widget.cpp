/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_normal_preview_widget.h"
#include <cmath>
#include <QColor>
#include <QPoint>

#include "kis_global.h"
#include <KoResourcePaths.h>

KisNormalPreviewWidget::KisNormalPreviewWidget(QWidget *parent)
        : QLabel(parent)
{
    m_redChannel = 0;
    m_greenChannel = 2;
    m_blueChannel = 4;
    //TODO: this can be changed in frameworks to  KoResourcePaths::findResource("kis_images", "krita-tangentnormal.png");
    m_fileName = KoResourcePaths::findResource("kis_images", "krita-tangentnormal-preview.png");
    QImage preview = QImage(m_fileName);
    setPixmap(QPixmap::fromImage(preview.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

KisNormalPreviewWidget::~KisNormalPreviewWidget()
{
}

void KisNormalPreviewWidget::setRedChannel(int index)
{
    if (index>=0 && index<6){
        m_redChannel=index;
    }
    updateImage();
}

void KisNormalPreviewWidget::setGreenChannel(int index)
{
    if (index>=0 && index<6){
        m_greenChannel=index;
    }
    updateImage();
}

void KisNormalPreviewWidget::setBlueChannel(int index)
{
    if (index>=0 && index<6){
        m_blueChannel=index;
    }
    updateImage();
}

void KisNormalPreviewWidget::updateImage()
{
    QImage preview = QImage(m_fileName);
    preview = swizzleTransformPreview (preview);
    setPixmap(QPixmap::fromImage(preview.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    repaint();
}

QImage KisNormalPreviewWidget::swizzleTransformPreview (QImage preview)
{
    int width = preview.width();
    int height = preview.height();
    QImage endPreview(preview.width(),preview.height(),QImage::Format_RGB32);
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            QColor currentcolor = QColor(preview.pixel(x,y));
            int r, g, b =0;
            r = previewTransform(currentcolor.red(), currentcolor.green(), currentcolor.blue(), m_redChannel, 255);
            g = previewTransform(currentcolor.red(), currentcolor.green(), currentcolor.blue(), m_greenChannel, 255);
            b = previewTransform(currentcolor.red(), currentcolor.green(), currentcolor.blue(), m_blueChannel, 255);
            QRgb transformedColor = qRgb(r,g,b);
            endPreview.setPixel(x,y, transformedColor);
        }
    }
    return endPreview;
}

int KisNormalPreviewWidget::previewTransform(int const horizontal, int const vertical, int const depth, int index, int maxvalue)
{
    int component = 0;
    switch(index) {
    case 0: component = horizontal; break;
    case 1: component = maxvalue-horizontal; break;
    case 2: component = vertical; break;
    case 3: component = maxvalue-vertical; break;
    case 4: component = depth; break;
    case 5: component = maxvalue-depth; break;
    }
    return component;
}
