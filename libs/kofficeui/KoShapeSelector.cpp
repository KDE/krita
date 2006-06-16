/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeSelector.h"

#include <KoShapeManager.h>
#include <KoPointerEvent.h>
//#include <KoTool.h>
#include <KoShapeRegistry.h>
#include <KoToolManager.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoInteractionTool.h>
#include <KoShapeMoveStrategy.h>
#include <KoCanvasController.h>

#include <QKeyEvent>
#include <QPainter>
#include <QMouseEvent>

#include <kdebug.h>
#include <kiconloader.h>

// ******** TemplateShape *********
/// \internal
class TemplateShape : public KoShape {
public:
    TemplateShape(KoShapeTemplate shapeTemplate) {
        m_icon = KGlobal::iconLoader()->loadIcon(shapeTemplate.icon, K3Icon::NoGroup, 22);
        resize(m_icon.size());
        m_shapeTemplate = shapeTemplate;
    }

    void paint(QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(converter);
        painter.drawPixmap(QRect( QPoint(0,0), m_icon.size()), m_icon);
    }

    KoShapeTemplate const *shapeTemplate() {
        return &m_shapeTemplate;
    }

private:
    QPixmap m_icon;
    KoShapeTemplate m_shapeTemplate;
};


// ******** GroupShape *********
/// \internal
class GroupShape : public KoShapeContainer {
public:
    GroupShape(KoShapeFactory *shapeFactory) {
        m_icon = KGlobal::iconLoader()->loadIcon(shapeFactory->icon(), K3Icon::NoGroup, 22);
        resize(m_icon.size());
        m_shapeFactory = shapeFactory;
    }

    void paintComponent(QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(converter);
        painter.drawPixmap(QRect( QPoint(0,0), m_icon.size()), m_icon);
    }

    const KoShapeFactory *shapeFactory() {
        return m_shapeFactory;
    }

private:
    QPixmap m_icon;
    KoShapeFactory *m_shapeFactory;
};


// ******* MoveTool *********
/// \internal
class MoveTool : public KoInteractionTool {
public:
    MoveTool(KoCanvasBase *canvas) : KoInteractionTool(canvas) {};
    void mousePressEvent( KoPointerEvent *event ) {
        KoShape *clickedShape = m_canvas->shapeManager()->getObjectAt(event->point);
        repaintDecorations();
        m_canvas->shapeManager()->selection()->deselectAll();
        if(clickedShape) {
            m_canvas->shapeManager()->selection()->select(clickedShape);
        }
        m_currentStrategy = new KoShapeMoveStrategy(this, m_canvas, event->point);
    }
    void mouseMoveEvent (KoPointerEvent *event) {
        KoInteractionTool::mouseMoveEvent(event);
        //m_canvas->shapeManager()->getObjectAt
    }
};


// ************** KoShapeSelector ************
KoShapeSelector::KoShapeSelector(QWidget *parent, KoCanvasController *cc, QString regExp)
: QWidget(parent)
, m_canvasController(cc)
{
    m_canvas = new Canvas(this);
    m_tool = new MoveTool(m_canvas);
    m_shapeManager = new KoShapeManager(m_canvas);
    setMinimumSize(20, 200);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    foreach(KoID id, KoShapeRegistry::instance()->listKeys()) {
        KoShapeFactory *factory = KoShapeRegistry::instance()->get(id);
        bool oneAdded=false;
        foreach(KoShapeTemplate shapeTemplate, factory->templates()) {
            oneAdded=true;
            TemplateShape *shape = new TemplateShape(shapeTemplate);
            add(shape);
        }
        if(!oneAdded)
            add(new GroupShape(factory));
    }

    connect(m_shapeManager->selection(), SIGNAL(selectionChanged()), this, SLOT(itemSelected()));
}

KoShapeSelector::~KoShapeSelector() {
    delete m_shapeManager;
    delete m_canvas;
}


void KoShapeSelector::itemSelected() {
    QList<KoShape*> allSelected = m_shapeManager->selection()->selectedObjects().toList();
    if(allSelected.isEmpty())
        return;
    KoShape *shape= allSelected.first();

    KoCreateShapesTool *tool = KoToolManager::instance()->shapeCreatorTool(m_canvasController->canvas());

    TemplateShape *templateShape = dynamic_cast<TemplateShape*> (shape);
    if(templateShape) {
        tool->setShapeId(templateShape->shapeTemplate()->id);
        tool->setShapeProperties(templateShape->shapeTemplate()->properties);
    }
    else {
        GroupShape *group = dynamic_cast<GroupShape*>(shape);
        tool->setShapeId(group->shapeFactory()->shapeId());
        tool->setShapeProperties(0);
    }
}

void KoShapeSelector::add(KoShape *shape) {
    int x=5, y=5; // 5 = gap
    int w = (int) shape->size().width();
    bool ok=true; // lets be optimistic ;)
    do {
        int rowHeight=0;
        ok=true;
        foreach(const KoShape *shape, m_shapeManager->objects()) {
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if(x + w > width()) {
                y += rowHeight + 5; // 5 = gap
                ok=false;
                break;
            }
        }
    } while(! ok);
    shape->setPosition(QPointF(x, y));

    m_shapeManager->add(shape);
}

// event handlers
void KoShapeSelector::mouseMoveEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( m_canvas->viewConverter()->viewToNormal(e->pos()) ));
    m_tool->mouseMoveEvent( &ev );
}

void KoShapeSelector::mousePressEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( m_canvas->viewConverter()->viewToNormal(e->pos()) ));
    m_tool->mousePressEvent( &ev );
}

void KoShapeSelector::mouseReleaseEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( m_canvas->viewConverter()->viewToNormal(e->pos()) ));
    m_tool->mouseReleaseEvent( &ev );
}

void KoShapeSelector::keyReleaseEvent (QKeyEvent *e) {
    m_tool->keyReleaseEvent(e);
}

void KoShapeSelector::keyPressEvent( QKeyEvent *e ) {
    m_tool->keyPressEvent(e);
}

void KoShapeSelector::paintEvent(QPaintEvent * ev) {
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(ev->rect());

    m_shapeManager->paint( painter, *(m_canvas->viewConverter()), false );
    painter.end();
}


// ************ DummyViewConverter **********
QPointF KoShapeSelector::DummyViewConverter::normalToView (const QPointF &normalPoint) {
    return normalPoint;
}

QPointF KoShapeSelector::DummyViewConverter::viewToNormal (const QPointF &viewPoint) {
    return viewPoint;
}

QRectF KoShapeSelector::DummyViewConverter::normalToView (const QRectF &normalRect) {
    return normalRect;
}

QRectF KoShapeSelector::DummyViewConverter::viewToNormal (const QRectF &viewRect) {
    return viewRect;
}

void KoShapeSelector::DummyViewConverter::zoom (double *zoomX, double *zoomY) const {
    *zoomX = 1.0;
    *zoomY = 1.0;
}


// ********* Canvas **********
KoShapeSelector::Canvas::Canvas(KoShapeSelector *parent)
: m_parent(parent)
{
}

void KoShapeSelector::Canvas::gridSize (double *horizontal, double *vertical) const {
}

void KoShapeSelector::Canvas::updateCanvas (const QRectF &rc) {
    QRect rect = rc.toRect();
    rect.adjust(-2, -2, 2, 2); // grow for to anti-aliasing
    m_parent->update(rect);
}

void  KoShapeSelector::Canvas::addCommand (KCommand *command, bool execute) {
    if(execute)
        command->execute();
    delete command;
};

#include "KoShapeSelector.moc"
