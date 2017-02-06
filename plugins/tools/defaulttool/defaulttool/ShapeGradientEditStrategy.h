/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SHAPEGRADIENTEDITSTRATEGY_H
#define SHAPEGRADIENTEDITSTRATEGY_H

#include <QScopedPointer>
#include <KoInteractionStrategy.h>
#include "KoShapeGradientHandles.h"

class ShapeGradientEditStrategy : public KoInteractionStrategy
{
public:
    ShapeGradientEditStrategy(KoToolBase *tool, KoShape *shape,
                              KoShapeGradientHandles::Handle::Type startHandleType,
                              const QPointF &clicked);
    ~ShapeGradientEditStrategy();

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // SHAPEGRADIENTEDITSTRATEGY_H
