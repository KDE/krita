/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoInteractionTool.h"
#include "KoInteractionTool_p.h"
#include "KoToolBase_p.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"

#include "kis_global.h"
#include "kis_assert.h"


KoInteractionTool::KoInteractionTool(KoCanvasBase *canvas)
    : KoToolBase(*(new KoInteractionToolPrivate(this, canvas)))
{
}

KoInteractionTool::~KoInteractionTool()
{
}

void KoInteractionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoInteractionTool);

    if (d->currentStrategy) {
        d->currentStrategy->paint(painter, converter);
    } else {
        Q_FOREACH (KoInteractionStrategyFactorySP factory, d->interactionFactories) {
            // skip the rest of rendering if the factory asks for it
            if (factory->paintOnHover(painter, converter)) break;
        }
    }
}

void KoInteractionTool::mousePressEvent(KoPointerEvent *event)
{
    Q_D(KoInteractionTool);
    if (d->currentStrategy) { // possible if the user presses an extra mouse button
        cancelCurrentStrategy();
        return;
    }
    d->currentStrategy = createStrategyBase(event);
    if (d->currentStrategy == 0)
        event->ignore();
}

void KoInteractionTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_D(KoInteractionTool);
    d->lastPoint = event->point;

    if (d->currentStrategy) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    } else {
        Q_FOREACH (KoInteractionStrategyFactorySP factory, d->interactionFactories) {
            // skip the rest of rendering if the factory asks for it
            if (factory->hoverEvent(event)) return;
        }

        event->ignore();
    }
}

void KoInteractionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoInteractionTool);
    if (d->currentStrategy) {
        d->currentStrategy->finishInteraction(event->modifiers());
        KUndo2Command *command = d->currentStrategy->createCommand();
        if (command)
            d->canvas->addCommand(command);
        delete d->currentStrategy;
        d->currentStrategy = 0;
        repaintDecorations();
    } else
        event->ignore();
}

void KoInteractionTool::keyPressEvent(QKeyEvent *event)
{
    Q_D(KoInteractionTool);
    event->ignore();
    if (d->currentStrategy &&
            (event->key() == Qt::Key_Control ||
             event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
             event->key() == Qt::Key_Meta)) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
        event->accept();
    }
}

void KoInteractionTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(KoInteractionTool);

    if (!d->currentStrategy) {
        KoToolBase::keyReleaseEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        cancelCurrentStrategy();
        event->accept();
    } else if (event->key() == Qt::Key_Control ||
               event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
               event->key() == Qt::Key_Meta) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    }
}

KoInteractionStrategy *KoInteractionTool::currentStrategy()
{
    Q_D(KoInteractionTool);
    return d->currentStrategy;
}

void KoInteractionTool::cancelCurrentStrategy()
{
    Q_D(KoInteractionTool);
    if (d->currentStrategy) {
        d->currentStrategy->cancelInteraction();
        delete d->currentStrategy;
        d->currentStrategy = 0;
    }
}

KoInteractionStrategy *KoInteractionTool::createStrategyBase(KoPointerEvent *event)
{
    Q_D(KoInteractionTool);

    Q_FOREACH (KoInteractionStrategyFactorySP factory, d->interactionFactories) {
        KoInteractionStrategy *strategy = factory->createStrategy(event);
        if (strategy) {
            return strategy;
        }
    }

    return createStrategy(event);
}

void KoInteractionTool::addInteractionFactory(KoInteractionStrategyFactory *factory)
{
    Q_D(KoInteractionTool);

    Q_FOREACH (auto f, d->interactionFactories) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(f->id() != factory->id());
    }

    d->interactionFactories.append(toQShared(factory));
    std::sort(d->interactionFactories.begin(),
          d->interactionFactories.end(),
          KoInteractionStrategyFactory::compareLess);
}

void KoInteractionTool::removeInteractionFactory(const QString &id)
{
    Q_D(KoInteractionTool);
    QList<KoInteractionStrategyFactorySP>::iterator it =
            d->interactionFactories.begin();

    while (it != d->interactionFactories.end()) {
        if ((*it)->id() == id) {
            it = d->interactionFactories.erase(it);
        } else {
            ++it;
        }
    }
}

bool KoInteractionTool::hasInteractioFactory(const QString &id)
{
    Q_D(KoInteractionTool);

    Q_FOREACH (auto f, d->interactionFactories) {
        if (f->id() == id) {
            return true;
        }
    }

    return false;
}

bool KoInteractionTool::tryUseCustomCursor()
{
    Q_D(KoInteractionTool);

    Q_FOREACH (auto f, d->interactionFactories) {
        if (f->tryUseCustomCursor()) {
            return true;
        }
    }

    return false;
}

KoInteractionTool::KoInteractionTool(KoInteractionToolPrivate &dd)
    : KoToolBase(dd)
{
}
