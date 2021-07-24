#ifndef KISREFERENCEIMAGECROPSTRATEGY_H
#define KISREFERENCEIMAGECROPSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <KisReferenceImage.h>
#include <KoToolBase.h>

class KoToolBase;

class KisReferenceImageCropStrategy : public KoInteractionStrategy
{
public:
    KisReferenceImageCropStrategy(KoToolBase *tool, KisReferenceImage *referenceImage, const QPointF &clicked, KoFlake::SelectionHandle direction);
    ~KisReferenceImageCropStrategy();

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    KisReferenceImage *m_referenceImage;
    QPointF m_start;
    bool m_top, m_left, m_bottom, m_right, m_move;
    QRectF m_initialCropRect;
    QTransform m_unwindMatrix;
    QPointF m_offset;
};

#endif // KISREFERENCEIMAGECROPSTRATEGY_H
