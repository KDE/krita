/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CUT_THROUGH_SHAPE_STRATEGY_H_
#define CUT_THROUGH_SHAPE_STRATEGY_H_

#include <QScopedPointer>
#include <QRectF>

#include <KoInteractionStrategy.h>
#include <KoShape.h>

class KoSelection;



class CutThroughShapeStrategy : public KoInteractionStrategy
{
public:
    CutThroughShapeStrategy(KoToolBase *tool, KoSelection *selection, QPointF startPoint, qreal width);
    ~CutThroughShapeStrategy() override;


    KUndo2Command *createCommand() override;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    QPointF m_startPoint = QPointF();
    QPointF m_endPoint = QPointF();
    QRectF m_previousLineDirtyRect = QRectF();
    QList<KoShape *> m_selectedShapes;
    qreal m_width {10};
};




#endif // CUT_THROUGH_SHAPE_STRATEGY_H_
