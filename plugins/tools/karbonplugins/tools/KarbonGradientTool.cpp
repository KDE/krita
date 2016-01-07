/* This file is part of the KDE project
 * Copyright (C) 2007-2008,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "KarbonGradientTool.h"
#include "KarbonGradientEditStrategy.h"
#include "KarbonCursor.h"

#include <KoGradientEditWidget.h>

#include <KoShape.h>
#include <KoCanvasBase.h>
#include <KoDocumentResourceManager.h>
#include <KoCanvasResourceManager.h>
#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeStrokeCommand.h>
#include <KoResourceServerProvider.h>
#include <KoGradientBackground.h>
#include <KoGradientHelper.h>
#include <KoShapeController.h>
#include <KoShapeBackground.h>
#include <KoResource.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>

#include <klocalizedstring.h>

#include <QPainter>

// helper function
GradientStrategy *createStrategy(KoShape *shape, const QGradient *gradient, GradientStrategy::Target target)
{
    if (!shape || ! gradient) {
        return 0;
    }

    if (gradient->type() == QGradient::LinearGradient) {
        return new LinearGradientStrategy(shape, static_cast<const QLinearGradient *>(gradient), target);
    } else if (gradient->type() == QGradient::RadialGradient) {
        return new RadialGradientStrategy(shape, static_cast<const QRadialGradient *>(gradient), target);
    } else if (gradient->type() == QGradient::ConicalGradient) {
        return new ConicalGradientStrategy(shape, static_cast<const QConicalGradient *>(gradient), target);
    } else {
        return 0;
    }
}

KarbonGradientTool::KarbonGradientTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_gradient(0)
    , m_currentStrategy(0)
    , m_hoverStrategy(0)
    , m_gradientWidget(0)
    , m_currentCmd(0)
    , m_oldSnapStrategies(0)
{
}

KarbonGradientTool::~KarbonGradientTool()
{
    delete m_gradient;
}

void KarbonGradientTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.setBrush(Qt::green);   //TODO make configurable
    painter.setPen(Qt::blue);   //TODO make configurable

    Q_FOREACH (GradientStrategy *strategy, m_strategies) {
        bool current = (strategy == m_currentStrategy);
        painter.save();
        if (current) {
            painter.setBrush(Qt::red);   //TODO make configurable
        }
        strategy->paint(painter, converter, current);
        painter.restore();
    }
}

void KarbonGradientTool::repaintDecorations()
{
    Q_FOREACH (GradientStrategy *strategy, m_strategies) {
        canvas()->updateCanvas(strategy->boundingRect(*canvas()->viewConverter()));
    }
}

void KarbonGradientTool::mousePressEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    if (!m_gradient) {
        return;
    }

    // do we have a selected gradient ?
    if (m_currentStrategy) {
        // now select whatever we hit
        if (m_currentStrategy->hitHandle(event->point, *canvas()->viewConverter(), true) ||
                m_currentStrategy->hitStop(event->point, *canvas()->viewConverter(), true) ||
                m_currentStrategy->hitLine(event->point, *canvas()->viewConverter(), true)) {
            m_currentStrategy->setEditing(true);
            m_currentStrategy->repaint(*canvas()->viewConverter());
            return;
        }
        m_currentStrategy->repaint(*canvas()->viewConverter());
    }
    // are we hovering over a gradient ?
    if (m_hoverStrategy) {
        // now select whatever we hit
        if (m_hoverStrategy->hitHandle(event->point, *canvas()->viewConverter(), true) ||
                m_hoverStrategy->hitStop(event->point, *canvas()->viewConverter(), true) ||
                m_hoverStrategy->hitLine(event->point, *canvas()->viewConverter(), true)) {
            m_currentStrategy = m_hoverStrategy;
            m_hoverStrategy = 0;
            m_currentStrategy->setEditing(true);
            m_currentStrategy->repaint(*canvas()->viewConverter());
            return;
        }
    }

    qreal grabDist = canvas()->viewConverter()->viewToDocumentX(GradientStrategy::grabSensitivity());
    QRectF roi(QPointF(), QSizeF(grabDist, grabDist));
    roi.moveCenter(event->point);
    // check if we are on a shape without a gradient yet
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(roi);
    KoSelection *selection = canvas()->shapeManager()->selection();

    KoGradientEditWidget::GradientTarget target = m_gradientWidget->target();

    GradientStrategy *newStrategy = 0;

    Q_FOREACH (KoShape *shape, shapes) {
        if (!selection->isSelected(shape)) {
            continue;
        }

        if (target == KoGradientEditWidget::FillGradient) {
            // target is fill so check the background style
            if (!dynamic_cast<KoGradientBackground *>(shape->background().data())) {
                QSharedPointer<KoGradientBackground>  fill(new KoGradientBackground(*m_gradient));
                m_currentCmd = new KoShapeBackgroundCommand(shape, fill);
                shape->setBackground(fill);
                newStrategy = createStrategy(shape, m_gradient, GradientStrategy::Fill);
            }
        } else {
            // target is stroke so check the stroke style
            KoShapeStroke *stroke = dynamic_cast<KoShapeStroke *>(shape->stroke());
            if (!stroke) {
                stroke = new KoShapeStroke(1.0);
                stroke->setLineBrush(QBrush(*m_gradient));
                m_currentCmd = new KoShapeStrokeCommand(shape, stroke);
                shape->setStroke(stroke);
                newStrategy = createStrategy(shape, m_gradient, GradientStrategy::Stroke);
                break;
            } else {
                Qt::BrushStyle style = stroke->lineBrush().style();
                if (style < Qt::LinearGradientPattern || style > Qt::RadialGradientPattern) {
                    KoShapeStroke *newStroke = new KoShapeStroke(*stroke);
                    newStroke->setLineBrush(QBrush(*m_gradient));
                    m_currentCmd = new KoShapeStrokeCommand(shape, newStroke);
                    stroke->setLineBrush(QBrush(*m_gradient));
                    newStrategy = createStrategy(shape, m_gradient, GradientStrategy::Stroke);
                    break;
                }
            }
        }
    }

    if (newStrategy) {
        m_currentStrategy = newStrategy;
        m_strategies.insert(m_currentStrategy->shape(), m_currentStrategy);
        m_currentStrategy->startDrawing(event->point);
    }
}

void KarbonGradientTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_hoverStrategy = 0;

    // do we have a selected gradient ?
    if (m_currentStrategy) {
        // are we editing the current selected gradient ?
        if (m_currentStrategy->isEditing()) {
            QPointF mousePos = event->point;
            // snap to bounding box when moving handles
            if (m_currentStrategy->selection() == GradientStrategy::Handle) {
                mousePos = canvas()->snapGuide()->snap(mousePos, event->modifiers());
            }

            m_currentStrategy->repaint(*canvas()->viewConverter());
            m_currentStrategy->handleMouseMove(mousePos, event->modifiers());
            m_currentStrategy->repaint(*canvas()->viewConverter());
            return;
        }
        // are we on a gradient handle ?
        else if (m_currentStrategy->hitHandle(event->point, *canvas()->viewConverter(), false)) {
            m_currentStrategy->repaint(*canvas()->viewConverter());
            useCursor(KarbonCursor::needleMoveArrow());
            emit statusTextChanged(i18n("Drag to move gradient position."));
            return;
        }
        // are we on a gradient stop handle ?
        else if (m_currentStrategy->hitStop(event->point, *canvas()->viewConverter(), false)) {
            m_currentStrategy->repaint(*canvas()->viewConverter());
            useCursor(KarbonCursor::needleMoveArrow());
            const QGradient *g = m_currentStrategy->gradient();
            if (g && g->stops().count() > 2) {
                emit statusTextChanged(i18n("Drag to move color stop. Double click to remove color stop."));
            } else {
                emit statusTextChanged(i18n("Drag to move color stop."));
            }
            return;
        }
        // are we near the gradient line ?
        else if (m_currentStrategy->hitLine(event->point, *canvas()->viewConverter(), false)) {
            m_currentStrategy->repaint(*canvas()->viewConverter());
            useCursor(Qt::SizeAllCursor);
            emit statusTextChanged(i18n("Drag to move gradient position. Double click to insert color stop."));
            return;
        }
    }

    // we have no selected gradient, so lets check if at least
    // the mouse hovers over another gradient (handles and line)

    // first check if we hit any handles
    Q_FOREACH (GradientStrategy *strategy, m_strategies) {
        if (strategy->hitHandle(event->point, *canvas()->viewConverter(), false)) {
            m_hoverStrategy = strategy;
            useCursor(KarbonCursor::needleMoveArrow());
            return;
        }
    }
    // now check if we hit any lines
    Q_FOREACH (GradientStrategy *strategy, m_strategies) {
        if (strategy->hitLine(event->point, *canvas()->viewConverter(), false)) {
            m_hoverStrategy = strategy;
            useCursor(Qt::SizeAllCursor);
            return;
        }
    }

    useCursor(KarbonCursor::needleArrow());
}

void KarbonGradientTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_UNUSED(event)
    // if we are editing, get out of edit mode and add a command to the stack
    if (m_currentStrategy) {
        KUndo2Command *cmd = m_currentStrategy->createCommand(m_currentCmd);
        canvas()->addCommand(m_currentCmd ? m_currentCmd : cmd);
        m_currentCmd = 0;
        if (m_gradientWidget) {
            m_gradientWidget->setGradient(*m_currentStrategy->gradient());
            if (m_currentStrategy->target() == GradientStrategy::Fill) {
                m_gradientWidget->setTarget(KoGradientEditWidget::FillGradient);
            } else {
                m_gradientWidget->setTarget(KoGradientEditWidget::StrokeGradient);
            }
            m_gradientWidget->setStopIndex(m_currentStrategy->selectedColorStop());
        }
        m_currentStrategy->setEditing(false);
    }
}

void KarbonGradientTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (!m_currentStrategy) {
        return;
    }

    canvas()->updateCanvas(m_currentStrategy->boundingRect(*canvas()->viewConverter()));

    if (m_currentStrategy->handleDoubleClick(event->point)) {
        KUndo2Command *cmd = m_currentStrategy->createCommand(m_currentCmd);
        canvas()->addCommand(m_currentCmd ? m_currentCmd : cmd);
        m_currentCmd = 0;
        if (m_gradientWidget) {
            m_gradientWidget->setGradient(*m_currentStrategy->gradient());
            if (m_currentStrategy->target() == GradientStrategy::Fill) {
                m_gradientWidget->setTarget(KoGradientEditWidget::FillGradient);
            } else {
                m_gradientWidget->setTarget(KoGradientEditWidget::StrokeGradient);
            }
        }
        canvas()->updateCanvas(m_currentStrategy->boundingRect(*canvas()->viewConverter()));
    }
}

void KarbonGradientTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_I: {
        uint handleRadius = GradientStrategy::handleRadius();
        if (event->modifiers() & Qt::ControlModifier) {
            handleRadius--;
        } else {
            handleRadius++;
        }
        // XXX: this is a KoDocumentResourceController feature, but shouldn't it be canvas?
        canvas()->shapeController()->resourceManager()->setHandleRadius(handleRadius);
    }
    break;
    default:
        event->ignore();
        return;
    }
    event->accept();
}

void KarbonGradientTool::activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes)
{
    Q_UNUSED(toolActivation);
    if (shapes.isEmpty()) {
        emit done();
        return;
    }

    initialize();
    repaintDecorations();

    useCursor(KarbonCursor::needleArrow());

    // save old enabled snap strategies, set bounding box snap strategy
    m_oldSnapStrategies = canvas()->snapGuide()->enabledSnapStrategies();
    canvas()->snapGuide()->enableSnapStrategies(KoSnapGuide::BoundingBoxSnapping);
    canvas()->snapGuide()->reset();

    connect(canvas()->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));
}

void KarbonGradientTool::initialize()
{
    if (m_currentStrategy && m_currentStrategy->isEditing()) {
        return;
    }

    m_hoverStrategy = 0;

    QList<KoShape *> selectedShapes = canvas()->shapeManager()->selection()->selectedShapes();
    QList<GradientStrategy *> strategies = m_strategies.values();
    // remove all gradient strategies no longer applicable
    Q_FOREACH (GradientStrategy *strategy, strategies) {
        // is this gradient shape still selected ?
        if (!selectedShapes.contains(strategy->shape()) || ! strategy->shape()->isEditable()) {
            m_strategies.remove(strategy->shape(), strategy);
            delete strategy;
            if (m_currentStrategy == strategy) {
                m_currentStrategy = 0;
            }
            continue;
        }
        // is the gradient a fill gradient but shape has no fill gradient anymore ?
        if (strategy->target() == GradientStrategy::Fill) {
            QSharedPointer<KoGradientBackground>  fill = qSharedPointerDynamicCast<KoGradientBackground>(strategy->shape()->background());
            if (!fill || ! fill->gradient() || fill->gradient()->type() != strategy->type()) {
                // delete the gradient
                m_strategies.remove(strategy->shape(), strategy);
                delete strategy;
                if (m_currentStrategy == strategy) {
                    m_currentStrategy = 0;
                }
                continue;
            }
        }
        // is the gradient a stroke gradient but shape has no stroke gradient anymore ?
        if (strategy->target() == GradientStrategy::Stroke) {
            KoShapeStroke *stroke = dynamic_cast<KoShapeStroke *>(strategy->shape()->stroke());
            if (!stroke  || ! stroke->lineBrush().gradient() || stroke->lineBrush().gradient()->type() != strategy->type()) {
                // delete the gradient
                m_strategies.remove(strategy->shape(), strategy);
                delete strategy;
                if (m_currentStrategy == strategy) {
                    m_currentStrategy = 0;
                }
                continue;
            }
        }
    }

    // now create new strategies if needed
    Q_FOREACH (KoShape *shape, selectedShapes) {
        if (!shape->isEditable()) {
            continue;
        }

        bool strokeExists = false;
        bool fillExists = false;
        // check which gradient strategies exist for this shape
        Q_FOREACH (GradientStrategy *strategy, m_strategies.values(shape)) {
            if (strategy->target() == GradientStrategy::Fill) {
                fillExists = true;
                strategy->updateStops();
            }
            if (strategy->target() == GradientStrategy::Stroke) {
                strokeExists = true;
                strategy->updateStops();
            }
        }

        if (!fillExists) {
            QSharedPointer<KoGradientBackground>  fill = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
            if (fill) {
                GradientStrategy *fillStrategy = createStrategy(shape, fill->gradient(), GradientStrategy::Fill);
                if (fillStrategy) {
                    m_strategies.insert(shape, fillStrategy);
                    fillStrategy->repaint(*canvas()->viewConverter());
                }
            }
        }

        if (!strokeExists) {
            KoShapeStroke *stroke = dynamic_cast<KoShapeStroke *>(shape->stroke());
            if (stroke) {
                GradientStrategy *strokeStrategy = createStrategy(shape, stroke->lineBrush().gradient(), GradientStrategy::Stroke);
                if (strokeStrategy) {
                    m_strategies.insert(shape, strokeStrategy);
                    strokeStrategy->repaint(*canvas()->viewConverter());
                }
            }
        }
    }

    if (m_strategies.count() == 0) {
        // create a default gradient
        m_gradient = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        m_gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
        m_gradient->setColorAt(0.0, Qt::white);
        m_gradient->setColorAt(1.0, Qt::green);
        return;
    }
    // automatically select strategy when editing single shape
    if (selectedShapes.count() == 1 && m_strategies.count()) {
        if (!m_currentStrategy || ! m_strategies.values().contains(m_currentStrategy)) {
            m_currentStrategy = m_strategies.values().first();
        }
    }

    delete m_gradient;
    GradientStrategy *strategy = m_currentStrategy ? m_currentStrategy : m_strategies.values().first();
    GradientStrategy::setHandleRadius(handleRadius());
    GradientStrategy::setGrabSensitivity(grabSensitivity());
    m_gradient = KoFlake::cloneGradient(strategy->gradient());
    if (m_gradientWidget) {
        if (m_gradient) {
            m_gradientWidget->setGradient(*m_gradient);
        }
        if (strategy->target() == GradientStrategy::Fill) {
            m_gradientWidget->setTarget(KoGradientEditWidget::FillGradient);
        } else {
            m_gradientWidget->setTarget(KoGradientEditWidget::StrokeGradient);
        }
    }
}

void KarbonGradientTool::deactivate()
{
    // we are not interested in selection content changes when not active
    disconnect(canvas()->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));

    delete m_gradient;
    m_gradient = 0;

    m_currentStrategy = 0;
    m_hoverStrategy = 0;
    qDeleteAll(m_strategies);
    m_strategies.clear();

    // restore previously set snap strategies
    canvas()->snapGuide()->enableSnapStrategies(m_oldSnapStrategies);
    canvas()->snapGuide()->reset();
}

void KarbonGradientTool::documentResourceChanged(int key, const QVariant &res)
{
    switch (key) {
    case KoDocumentResourceManager::HandleRadius:
        Q_FOREACH (GradientStrategy *strategy, m_strategies) {
            strategy->repaint(*canvas()->viewConverter());
        }
        GradientStrategy::setHandleRadius(res.toUInt());
        Q_FOREACH (GradientStrategy *strategy, m_strategies) {
            strategy->repaint(*canvas()->viewConverter());
        }
        break;
    case KoDocumentResourceManager::GrabSensitivity:
        GradientStrategy::setGrabSensitivity(res.toUInt());
        break;
    default:
        return;
    }
}

QList<QPointer<QWidget> > KarbonGradientTool::createOptionWidgets()
{
    m_gradientWidget = new KoGradientEditWidget();
    if (m_gradient) {
        m_gradientWidget->setGradient(*m_gradient);
    }

    connect(m_gradientWidget, SIGNAL(changed()), this, SLOT(gradientChanged()));

    KoResourceServer<KoAbstractGradient> *rserver = KoResourceServerProvider::instance()->gradientServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KoAbstractGradient>(rserver));
    KoResourceItemChooser *chooser = new KoResourceItemChooser(adapter, m_gradientWidget);
    chooser->setObjectName("KarbonGradientChooser");
    chooser->setColumnCount(1);

    connect(chooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(gradientSelected(KoResource*)));

    QList<QPointer<QWidget> > widgets;
    m_gradientWidget->setWindowTitle(i18n("Edit Gradient"));
    widgets.append(m_gradientWidget);
    chooser->setWindowTitle(i18n("Predefined Gradients"));
    widgets.append(chooser);

    return widgets;
}

void KarbonGradientTool::gradientSelected(KoResource *resource)
{
    if (!resource) {
        return;
    }

    KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient *>(resource);
    if (!gradient) {
        return;
    }

    QGradient *newGradient = gradient->toQGradient();
    if (newGradient) {
        m_gradientWidget->setGradient(*newGradient);
        gradientChanged();
        delete newGradient;
    }
}

void KarbonGradientTool::gradientChanged()
{
    QList<KoShape *> selectedShapes = canvas()->shapeManager()->selection()->selectedShapes();

    QGradient::Type type = m_gradientWidget->type();
    QGradient::Spread spread = m_gradientWidget->spread();
    QGradientStops stops = m_gradientWidget->stops();

    if (m_gradientWidget->target() == KoGradientEditWidget::FillGradient) {
        QList<QSharedPointer<KoShapeBackground> > newFills;
        Q_FOREACH (KoShape *shape, selectedShapes) {
            QSharedPointer<KoGradientBackground> newFill;
            QSharedPointer<KoGradientBackground> oldFill = qSharedPointerDynamicCast<KoGradientBackground>(shape->background());
            if (oldFill) {
                QGradient *g = KoGradientHelper::convertGradient(oldFill->gradient(), type);
                g->setSpread(spread);
                g->setStops(stops);
                newFill = QSharedPointer<KoGradientBackground>(new KoGradientBackground(g, oldFill->transform()));
            } else {
                QGradient *g = KoGradientHelper::defaultGradient(type, spread, stops);
                newFill = QSharedPointer<KoGradientBackground>(new KoGradientBackground(g));
            }
            newFills.append(newFill);
        }
        canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, newFills));
    } else {
        QList<KoShapeStrokeModel *> newStrokes;
        Q_FOREACH (KoShape *shape, selectedShapes) {
            KoShapeStroke *stroke = dynamic_cast<KoShapeStroke *>(shape->stroke());
            KoShapeStroke *newStroke = 0;
            if (stroke) {
                newStroke = new KoShapeStroke(*stroke);
            } else {
                newStroke = new KoShapeStroke(1.0);
            }
            QBrush newGradient;
            if (newStroke->lineBrush().gradient()) {
                QGradient *g = KoGradientHelper::convertGradient(newStroke->lineBrush().gradient(), type);
                g->setSpread(spread);
                g->setStops(stops);
                newGradient = QBrush(*g);
                delete g;
            } else {
                QGradient *g = KoGradientHelper::defaultGradient(type, spread, stops);
                newGradient = QBrush(*g);
                delete g;
            }
            newStroke->setLineBrush(newGradient);
            newStrokes.append(newStroke);
        }
        canvas()->addCommand(new KoShapeStrokeCommand(selectedShapes, newStrokes));
    }
    initialize();
}

