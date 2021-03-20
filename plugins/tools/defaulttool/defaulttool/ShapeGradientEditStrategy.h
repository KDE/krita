/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SHAPEGRADIENTEDITSTRATEGY_H
#define SHAPEGRADIENTEDITSTRATEGY_H

#include <QScopedPointer>
#include <KoInteractionStrategy.h>
#include "KoShapeGradientHandles.h"

class ShapeGradientEditStrategy : public KoInteractionStrategy
{
public:
    ShapeGradientEditStrategy(KoToolBase *tool,
                              KoFlake::FillVariant fillVariant,
                              KoShape *shape,
                              KoShapeGradientHandles::Handle::Type startHandleType,
                              const QPointF &clicked);
    ~ShapeGradientEditStrategy() override;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // SHAPEGRADIENTEDITSTRATEGY_H
