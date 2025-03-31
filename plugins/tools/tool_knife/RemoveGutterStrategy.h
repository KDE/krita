/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef REMOVEGUTTERSTRATEGY_H
#define REMOVEGUTTERSTRATEGY_H

#include <QScopedPointer>
#include <QRectF>

#include <KoInteractionStrategy.h>
#include <KoShape.h>

class KoSelection;


class RemoveGutterStrategy : public KoInteractionStrategy
{
public:
    RemoveGutterStrategy(KoToolBase *tool, KoSelection *selection, const QList<KoShape*> &shapes, QPointF startPoint);
    ~RemoveGutterStrategy() override;

    KUndo2Command *createCommand() override;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    QPointF m_startPoint = QPointF();
    QPointF m_endPoint = QPointF();
    QRectF m_previousLineDirtyRect = QRectF();

    QList<KoShape *> m_allShapes;
    QList<KoShape *> m_selectedShapes;

};

#endif // REMOVEGUTTERSTRATEGY_H
