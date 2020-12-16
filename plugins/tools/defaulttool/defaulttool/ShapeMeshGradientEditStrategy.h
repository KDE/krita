/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
