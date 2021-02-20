/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOPARAMETERCHANGESTRATEGY_H
#define KOPARAMETERCHANGESTRATEGY_H

#include "kritaflake_export.h"
#include "KoInteractionStrategy.h"
#include <QPointF>

class KoParameterShape;
class KoParameterChangeStrategyPrivate;

/// Strategy for changing control points of parametric shapes
class KRITAFLAKE_EXPORT KoParameterChangeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructs a strategy for changing control points of parametric shapes.
     * @param tool the tool the strategy belongs to
     * @param parameterShape the parametric shapes the strategy is working on
     * @param handleId the id of the handle to modify
     */
    KoParameterChangeStrategy(KoToolBase *tool, KoParameterShape *parameterShape, int handleId);
    ~KoParameterChangeStrategy() override;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    KUndo2Command* createCommand() override;

protected:
    /// constructor
    KoParameterChangeStrategy(KoParameterChangeStrategyPrivate &);

private:
    Q_DECLARE_PRIVATE(KoParameterChangeStrategy)
};

#endif /* KOPARAMETERCHANGESTRATEGY_H */
