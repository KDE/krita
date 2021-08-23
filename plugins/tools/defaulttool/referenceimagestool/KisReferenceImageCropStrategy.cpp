/*
 *  SPDX-FileCopyrightText: 2021 Sachin Jindal <jindalsachin01@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisReferenceImageCropStrategy.h"
#include <KoFlake.h>
#include <KoSnapGuide.h>
#include "ToolReferenceImages.h"

KisReferenceImageCropStrategy::KisReferenceImageCropStrategy(KoToolBase *tool, KisReferenceImage *referenceImage, const QPointF &clicked, KoFlake::SelectionHandle direction)
    : KoInteractionStrategy(tool)
{
    QTransform matrix = referenceImage->absoluteTransformation();
    QPolygonF cropRect = matrix.map(QPolygonF(referenceImage->cropRect()));
    m_move = false;

    switch (direction) {
    case KoFlake::TopMiddleHandle:
        m_start = 0.5 * (cropRect.value(0) + cropRect.value(1));
        m_top = true; m_bottom = false; m_left = false; m_right = false;
        break;
    case KoFlake::TopRightHandle:
        m_start = cropRect.value(1);
        m_top = true; m_bottom = false; m_left = false; m_right = true;
        break;
    case KoFlake::RightMiddleHandle:
        m_start = 0.5 * (cropRect.value(1) + cropRect.value(2));
        m_top = false; m_bottom = false; m_left = false; m_right = true;
        break;
    case KoFlake::BottomRightHandle:
        m_start = cropRect.value(2);
        m_top = false; m_bottom = true; m_left = false; m_right = true;
        break;
    case KoFlake::BottomMiddleHandle:
        m_start = 0.5 * (cropRect.value(2) + cropRect.value(3));
        m_top = false; m_bottom = true; m_left = false; m_right = false;
        break;
    case KoFlake::BottomLeftHandle:
        m_start = cropRect.value(3);
        m_top = false; m_bottom = true; m_left = true; m_right = false;
        break;
    case KoFlake::LeftMiddleHandle:
        m_start = 0.5 * (cropRect.value(3) + cropRect.value(0));
        m_top = false; m_bottom = false; m_left = true; m_right = false;
        break;
    case KoFlake::TopLeftHandle:
        m_start = cropRect.value(0);
        m_top = true; m_bottom = false; m_left = true; m_right = false;
        break;
    case KoFlake::NoHandle:
        m_start = clicked;
        m_move = true;
    }

    m_initialCropRect = referenceImage->cropRect();
    m_referenceImage = referenceImage;
    m_unwindMatrix = referenceImage->absoluteTransformation().inverted();
}

KisReferenceImageCropStrategy::~KisReferenceImageCropStrategy()
{
}

void KisReferenceImageCropStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    QRectF finalRect = m_initialCropRect;
    QPointF distance = m_unwindMatrix.map(point) - m_unwindMatrix.map(m_start);
    if(m_move) {
        QPointF offset = finalRect.topLeft() + distance;
        if(offset.x() < 0) {
            offset.setX(0);
        }
        if(offset.y() < 0) {
            offset.setY(0);
        }

        qreal maxX = m_referenceImage->size().width() - finalRect.width();
        if(offset.x() > maxX) {
            offset.setX(maxX);
        }
        qreal maxY = m_referenceImage->size().height() - finalRect.height();
        if(offset.y() > maxY) {
            offset.setY(maxY);
        }
        finalRect.moveTo(offset);
    }
    else {
        qreal newWidth = m_initialCropRect.width();
        qreal newHeight = m_initialCropRect.height();
        QPointF pos;

        if (m_left) {
            newWidth = m_initialCropRect.width() - distance.x();
            if(newWidth > 1) {
                pos.setX(distance.x());
            }
            qreal maxWidth = finalRect.bottomRight().x();
            if(newWidth > maxWidth) {
                newWidth = maxWidth;
            }
        } else if (m_right) {
            newWidth = m_initialCropRect.width() + distance.x();

            qreal maxWidth = m_referenceImage->size().width() - finalRect.topLeft().x();
            if(newWidth > maxWidth) {
                newWidth = maxWidth;
            }
        }

        if (m_top) {
            newHeight = m_initialCropRect.height() - distance.y();

            if(newHeight > 1) {
                pos.setY(distance.y());
            }
            qreal maxHeight = finalRect.bottomRight().y();
            if(newHeight > maxHeight) {
                newHeight = maxHeight;
            }
        } else if (m_bottom) {
            newHeight = m_initialCropRect.height() + distance.y();

            qreal maxHeight = m_referenceImage->size().height() - finalRect.topLeft().y();
            if(newHeight > maxHeight) {
                newHeight = maxHeight;
            }
        }
        QPointF offset = m_initialCropRect.topLeft() + pos;
        if(offset.x() < 0 ) {
            offset.setX(0);
        }
        if(offset.y() < 0) {
            offset.setY(0);
        }
        if(newWidth < 1) {
            newWidth = 1;
        }
        if(newHeight < 1) {
            newHeight = 1;
        }
        finalRect.moveTo(offset);
        finalRect.setWidth(newWidth);
        finalRect.setHeight(newHeight);
    }
    m_referenceImage->setCropRect(finalRect);

    ToolReferenceImages *refTool = dynamic_cast<ToolReferenceImages*>(tool());
    emit refTool->cropRectChanged();

    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(tool()->canvas());
    if(kisCanvas) {
        QRectF rect = kisCanvas->coordinatesConverter()->widgetToDocument(m_referenceImage->boundingRect());
        m_referenceImage->updateAbsolute(rect);
    }
}

KUndo2Command *KisReferenceImageCropStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();
    return 0;
}

void KisReferenceImageCropStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

void KisReferenceImageCropStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

