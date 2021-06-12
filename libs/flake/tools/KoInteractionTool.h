/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOINTERACTIONTOOL_H
#define KOINTERACTIONTOOL_H

#include "KoToolBase.h"
#include "kritaflake_export.h"

class KoInteractionStrategy;
class KoInteractionStrategyFactory;
class KoInteractionToolPrivate;

#define KoInteractionTool_ID "InteractionTool"

/**
 * The interaction tool adds to the normal KoToolBase class the concept of strategies
 * as a means to get one tool to have different actions the user can perform using the mouse.
 * Each time the user presses the mouse until she releases the mouse a strategy object
 * will be created, used and discarded.
 * If the usage of a tool fits this pattern you need to inherit from this class instead of the
 * plain KoToolBase and reimplement your createStrategy() method which returns a tool-specific
 * strategy where all the real interaction code is placed.
 * A tool can then become as simple as this;
 * @code
    class MyTool : public KoInteractionTool
    {
    public:
        MyTool::MyTool(KoCanvasBase *canvas) : KoInteractionTool( canvas ) { }

        KoInteractionStrategy *MyTool::createStrategy(KoPointerEvent *event) {
            return new MyStrategy(this, m_canvas, event->point);
        }
    };
 * @endcode
 * Whereas your strategy (MyStrategy in the example) will contain the interaction code.
 */
class KRITAFLAKE_EXPORT KoInteractionTool : public KoToolBase
{
    Q_OBJECT
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KoInteractionStrategy.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KoInteractionTool(KoCanvasBase *canvas);
    ~KoInteractionTool() override;

public:
    void paint(QPainter &painter, const KoViewConverter &converter) override;

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

protected:
    /// \internal
    KoInteractionTool(KoInteractionToolPrivate &dd);

    KoInteractionStrategy *currentStrategy(); ///< the strategy that is 'in progress'
    /// Cancels the current strategy and deletes it.
    void cancelCurrentStrategy();

    /**
     * Reimplement this factory method to create your strategy to be used for mouse interaction.
     * @returns a new strategy, or 0 when there is nothing to do.
     */
    KoInteractionStrategy *createStrategyBase(KoPointerEvent *event);
    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event) = 0;

    void addInteractionFactory(KoInteractionStrategyFactory *factory);
    void removeInteractionFactory(const QString &id);
    bool hasInteractioFactory(const QString &id);

    bool tryUseCustomCursor();

private:
    KoInteractionTool(const KoInteractionTool&);
    KoInteractionTool& operator=(const KoInteractionTool&);

    Q_DECLARE_PRIVATE(KoInteractionTool)
};

#endif /* KOINTERACTIONTOOL_H */
