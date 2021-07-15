#include "KisReferenceImageCropStrategy.h"
#include <KoFlake.h>
#include <KoSnapGuide.h>
#include "ToolReferenceImages.h"

KisReferenceImageCropStrategy::KisReferenceImageCropStrategy(KoToolBase *tool, KisReferenceImage *referenceImage, const QPointF &clicked, KoFlake::SelectionHandle direction)
    : KoInteractionStrategy(tool)
{
    QTransform matrix = referenceImage->absoluteTransformation();
    QPolygonF cropRect = matrix.map(QPolygonF(referenceImage->cropRect()));

    switch (direction) {
    case KoFlake::TopMiddleHandle:
        m_start = 0.5 * (cropRect.value(0) + cropRect.value(1)) / 2;
        m_top = true; m_bottom = false; m_left = false; m_right = false;
        break;
    case KoFlake::TopRightHandle:
        m_start = cropRect.value(1);
        m_top = true; m_bottom = false; m_left = false; m_right = true;
        break;
    case KoFlake::RightMiddleHandle:
        m_start = 0.5 * (cropRect.value(1) + cropRect.value(2)) / 2;
        m_top = false; m_bottom = false; m_left = false; m_right = true;
        break;
    case KoFlake::BottomRightHandle:
        m_start = cropRect.value(2);
        m_top = false; m_bottom = true; m_left = false; m_right = true;
        break;
    case KoFlake::BottomMiddleHandle:
        m_start = 0.5 * (cropRect.value(2) + cropRect.value(3)) / 2;
        m_top = false; m_bottom = true; m_left = false; m_right = false;
        break;
    case KoFlake::BottomLeftHandle:
        m_start = cropRect.value(3);
        m_top = false; m_bottom = true; m_left = true; m_right = false;
        break;
    case KoFlake::LeftMiddleHandle:
        m_start = 0.5 * (cropRect.value(3) + cropRect.value(0)) / 2;
        m_top = false; m_bottom = false; m_left = true; m_right = false;
        break;
    case KoFlake::TopLeftHandle:
        m_start = cropRect.value(0);
        m_top = true; m_bottom = false; m_left = true; m_right = false;
        break;
    case KoFlake::NoHandle:
        m_start = clicked;
        m_move = true;
    //handle illegal corner
    }

    m_initialCropRect = referenceImage->cropRect();
    m_referenceImage = referenceImage;
    m_unwindMatrix = referenceImage->absoluteTransformation().inverted();
    offset = m_referenceImage->cropRect().topLeft();
}

KisReferenceImageCropStrategy::~KisReferenceImageCropStrategy()
{

}


void KisReferenceImageCropStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    QRectF finalRect = m_initialCropRect;
    if(m_move) {
        QPointF newPos = point - m_start;
        finalRect.moveTo(newPos);
    }
    else {
        QPointF newPos = tool()->canvas()->snapGuide()->snap(point, modifiers);
        QPointF distance = m_unwindMatrix.map(newPos) - m_unwindMatrix.map(m_start);

        qreal startWidth = m_initialCropRect.width();
        qreal startHeight = m_initialCropRect.height();
        qreal newWidth = startWidth;
        qreal newHeight = startHeight;
        QPointF pos;

        if (m_left) {
            pos.setX(newPos.x() - m_start.x());
            newWidth = startWidth - distance.x();
        } else if (m_right) {
            newWidth = startWidth + distance.x();
        }

        if (m_top) {
            pos.setY(newPos.y() - m_start.y());
            newHeight = startHeight - distance.y();

        } else if (m_bottom) {
            newHeight = startHeight + distance.y();
        }

        finalRect.moveTo(offset + pos);
        finalRect.setWidth(newWidth);
        finalRect.setHeight(newHeight);
    }
    m_referenceImage->setCropRect(finalRect);
    m_referenceImage->update();
    ToolReferenceImages *t = dynamic_cast<ToolReferenceImages*>(tool());
    emit t->cropRectChanged();

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

