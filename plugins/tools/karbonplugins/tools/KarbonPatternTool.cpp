/* This file is part of the KDE project
 * Copyright (C) 2007,2009,2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonPatternTool.h"
#include "KarbonPatternEditStrategy.h"
#include <KarbonPatternOptionsWidget.h>

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoShape.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeBackgroundCommand.h>
#include <KoPointerEvent.h>
#include <resources/KoPattern.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoShapeController.h>
#include <resources/KoResource.h>
#include <KoResourceServerProvider.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>

#include <klocalizedstring.h>

#include <QPainter>
#include <QWidget>
#include <kundo2command.h>

KarbonPatternTool::KarbonPatternTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_currentStrategy(0)
    , m_optionsWidget(0)
{
}

KarbonPatternTool::~KarbonPatternTool()
{
}

void KarbonPatternTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.setBrush(Qt::green);   //TODO make configurable
    painter.setPen(Qt::blue);   //TODO make configurable
    painter.setTransform(converter.documentToView(), true);

    // paint all the strategies
    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        if (strategy == m_currentStrategy) {
            continue;
        }

        painter.save();
        strategy->paint(painter, converter);
        painter.restore();
    }

    // paint selected strategy with another color
    if (m_currentStrategy) {
        painter.setBrush(Qt::red);   //TODO make configurable
        m_currentStrategy->paint(painter, converter);
    }
}

void KarbonPatternTool::repaintDecorations()
{
    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        canvas()->updateCanvas(strategy->boundingRect());
    }
}

void KarbonPatternTool::mousePressEvent(KoPointerEvent *event)
{
    //m_currentStrategy = 0;

    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        if (strategy->selectHandle(event->point, *canvas()->viewConverter())) {
            m_currentStrategy = strategy;
            m_currentStrategy->repaint();
            useCursor(Qt::SizeAllCursor);
            break;
        }
    }
    if (m_currentStrategy) {
        m_currentStrategy->setEditing(true);
        updateOptionsWidget();
    }
}

void KarbonPatternTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        m_currentStrategy->repaint();
        if (m_currentStrategy->isEditing()) {
            m_currentStrategy->handleMouseMove(event->point, event->modifiers());
            m_currentStrategy->repaint();
            return;
        }
    }
    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        if (strategy->selectHandle(event->point, *canvas()->viewConverter())) {
            useCursor(Qt::SizeAllCursor);
            return;
        }
    }
    useCursor(Qt::ArrowCursor);
}

void KarbonPatternTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_UNUSED(event)
    // if we are editing, get out of edit mode and add a command to the stack
    if (m_currentStrategy && m_currentStrategy->isEditing()) {
        m_currentStrategy->setEditing(false);
        KUndo2Command *cmd = m_currentStrategy->createCommand();
        if (cmd) {
            canvas()->addCommand(cmd);
        }

        updateOptionsWidget();
    }
}

void KarbonPatternTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_I: {
        KoDocumentResourceManager *rm = canvas()->shapeController()->resourceManager();
        uint handleRadius = rm->handleRadius();
        if (event->modifiers() & Qt::ControlModifier) {
            handleRadius--;
        } else {
            handleRadius++;
        }
        rm->setHandleRadius(handleRadius);
    }
    break;
    default:
        event->ignore();
        return;
    }
    event->accept();
}

void KarbonPatternTool::initialize()
{
    if (m_currentStrategy && m_currentStrategy->isEditing()) {
        return;
    }

    QList<KoShape *> selectedShapes = canvas()->shapeManager()->selection()->selectedShapes();

    // remove all pattern strategies no longer applicable
    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        // is this gradient shape still selected ?
        if (!selectedShapes.contains(strategy->shape()) || ! strategy->shape()->isShapeEditable()) {
            m_strategies.remove(strategy->shape());
            if (m_currentStrategy == strategy) {
                m_currentStrategy = 0;
            }
            delete strategy;
            continue;
        }

        // does the shape has no fill pattern anymore ?
        QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(strategy->shape()->background());
        if (!fill) {
            // delete the gradient
            m_strategies.remove(strategy->shape());
            if (m_currentStrategy == strategy) {
                m_currentStrategy = 0;
            }
            delete strategy;
            continue;
        }

        strategy->updateHandles();
        strategy->repaint();
    }

    KoImageCollection *imageCollection = canvas()->shapeController()->resourceManager()->imageCollection();

    // now create new strategies if needed
    Q_FOREACH (KoShape *shape, selectedShapes) {
        if (!shape->isShapeEditable()) {
            continue;
        }

        // do we already have a strategy for that shape?
        if (m_strategies.contains(shape)) {
            continue;
        }

        if (qSharedPointerDynamicCast<KoPatternBackground>(shape->background())) {
            KarbonPatternEditStrategyBase *s = new KarbonOdfPatternEditStrategy(shape, imageCollection);
            m_strategies.insert(shape, s);
            s->repaint();
        }
    }
    // automatically select strategy when editing single shape
    if (m_strategies.count() == 1 && ! m_currentStrategy) {
        m_currentStrategy = m_strategies.begin().value();
        updateOptionsWidget();
    }

    if (m_currentStrategy) {
        m_currentStrategy->repaint();
    }
}

void KarbonPatternTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

    if (shapes.isEmpty()) {
        emit done();
        return;
    }

    initialize();

    KarbonPatternEditStrategyBase::setHandleRadius(handleRadius());
    KarbonPatternEditStrategyBase::setGrabSensitivity(grabSensitivity());

    useCursor(Qt::ArrowCursor);

    connect(canvas()->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));
}

void KarbonPatternTool::deactivate()
{
    // we are not interested in selection content changes when not active
    disconnect(canvas()->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));

    Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
        strategy->repaint();
    }

    qDeleteAll(m_strategies);
    m_strategies.clear();

    Q_FOREACH (KoShape *shape, canvas()->shapeManager()->selection()->selectedShapes()) {
        shape->update();
    }

    m_currentStrategy = 0;

    KoToolBase::deactivate();
}

void KarbonPatternTool::documentResourceChanged(int key, const QVariant &res)
{
    switch (key) {
    case KoDocumentResourceManager::HandleRadius:
        Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
            strategy->repaint();
        }

        KarbonPatternEditStrategyBase::setHandleRadius(res.toUInt());

        Q_FOREACH (KarbonPatternEditStrategyBase *strategy, m_strategies) {
            strategy->repaint();
        }
        break;
    case KoDocumentResourceManager::GrabSensitivity:
        KarbonPatternEditStrategyBase::setGrabSensitivity(res.toUInt());
        break;
    default:
        return;
    }
}

QList<QPointer<QWidget> > KarbonPatternTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgets;

    m_optionsWidget = new KarbonPatternOptionsWidget();
    connect(m_optionsWidget, SIGNAL(patternChanged()),
            this, SLOT(patternChanged()));

    KoResourceServer<KoPattern> *rserver = KoResourceServerProvider::instance()->patternServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KoPattern>(rserver));
    KoResourceItemChooser *chooser = new KoResourceItemChooser(adapter, m_optionsWidget);
    chooser->setObjectName("KarbonPatternChooser");

    connect(chooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(patternSelected(KoResource*)));

    m_optionsWidget->setWindowTitle(i18n("Pattern Options"));
    widgets.append(m_optionsWidget);
    chooser->setWindowTitle(i18n("Patterns"));
    widgets.append(chooser);
    updateOptionsWidget();
    return widgets;
}

void KarbonPatternTool::patternSelected(KoResource *resource)
{
    KoPattern *currentPattern = dynamic_cast<KoPattern *>(resource);
    if (!currentPattern || ! currentPattern->valid()) {
        return;
    }

    KoImageCollection *imageCollection = canvas()->shapeController()->resourceManager()->imageCollection();
    if (imageCollection) {
        QList<KoShape *> selectedShapes = canvas()->shapeManager()->selection()->selectedShapes();
        QSharedPointer<KoPatternBackground> newFill(new KoPatternBackground(imageCollection));
        newFill->setPattern(currentPattern->pattern());
        canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, newFill));
        initialize();
    }
}

void KarbonPatternTool::updateOptionsWidget()
{
    if (m_optionsWidget && m_currentStrategy) {
        QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(m_currentStrategy->shape()->background());
        if (fill) {
            m_optionsWidget->setRepeat(fill->repeat());
            m_optionsWidget->setReferencePoint(fill->referencePoint());
            m_optionsWidget->setReferencePointOffset(fill->referencePointOffset());
            m_optionsWidget->setTileRepeatOffset(fill->tileRepeatOffset());
            m_optionsWidget->setPatternSize(fill->patternDisplaySize().toSize());
        }
    }
}

void KarbonPatternTool::patternChanged()
{
    if (m_currentStrategy) {
        KoShape *shape = m_currentStrategy->shape();
        QSharedPointer<KoPatternBackground>  oldFill = qSharedPointerDynamicCast<KoPatternBackground>(shape->background());
        if (!oldFill) {
            return;
        }
        KoImageCollection *imageCollection = canvas()->shapeController()->resourceManager()->imageCollection();
        if (!imageCollection) {
            return;
        }
        QSharedPointer<KoPatternBackground> newFill(new KoPatternBackground(imageCollection));
        if (!newFill) {
            return;
        }
        newFill->setTransform(oldFill->transform());
        newFill->setPattern(oldFill->pattern());

        newFill->setRepeat(m_optionsWidget->repeat());
        newFill->setReferencePoint(m_optionsWidget->referencePoint());
        newFill->setReferencePointOffset(m_optionsWidget->referencePointOffset());
        newFill->setTileRepeatOffset(m_optionsWidget->tileRepeatOffset());
        newFill->setPatternDisplaySize(m_optionsWidget->patternSize());
        canvas()->addCommand(new KoShapeBackgroundCommand(shape, newFill));
    }
}

