/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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

#ifndef __SHAPEMESHGRADIENTEDITSTRATEGY_H_
#define __SHAPEMESHGRADIENTEDITSTRATEGY_H_

#include <KoInteractionStrategy.h>

#include "KoShapeMeshGradientHandles.h"

class ShapeMeshGradientEditStrategy : public KoInteractionStrategy {
public:
    ShapeMeshGradientEditStrategy(KoToolBase *tool,
                                  KoFlake::FillVariant fillVariant,
                                  KoShape *shape,
                                  KoShapeMeshGradientHandles::Handle startHandle,
                                  const QPointF &clicked);

    ~ShapeMeshGradientEditStrategy();

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;

    KUndo2Command* createCommand() override;

    void finishInteraction(Qt::KeyboardModifiers modifiers) override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // __SHAPEMESHGRADIENTEDITSTRATEGY_H_
