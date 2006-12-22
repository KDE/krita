/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeSelector.h"

#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoShapeRegistry.h>
#include <KoToolManager.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoInteractionTool.h>
#include <KoShapeMoveStrategy.h>
#include <KoCreateShapesTool.h>
#include <KoShapeController.h>
#include <KoCanvasController.h>
#include <KoProperties.h>

#include <QKeyEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QPointF>
#include <QToolTip>
#include <QTimer>
#include <QPainterPath>
#include <QUndoCommand>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>

// ******** IconShape *********
/// \internal
class IconShape : public KoShape {
public:
    IconShape(const QString &icon) {
        m_icon = KIcon(icon).pixmap(22);
        resize(m_icon.size());
    }

    virtual void visit(KoCreateShapesTool *tool) = 0;
    virtual QString toolTip() = 0;

    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(converter);
        painter.drawPixmap(QRect( QPoint(0,0), m_icon.size()), m_icon);
    }

    QPixmap pixmap() const { return m_icon; }

private:
    QPixmap m_icon;
};


// ******** TemplateShape *********
/// \internal
class TemplateShape : public IconShape {
public:
    TemplateShape(KoShapeTemplate shapeTemplate) : IconShape(shapeTemplate.icon) {
        m_shapeTemplate = shapeTemplate;
    }

    void visit(KoCreateShapesTool *tool) {
        tool->setShapeId(m_shapeTemplate.id);
        tool->setShapeProperties(m_shapeTemplate.properties);
    }

    QString toolTip() {
        return m_shapeTemplate.toolTip;
    }

    const KoShapeTemplate &shapeTemplate() const { return m_shapeTemplate; }

private:
    KoShapeTemplate m_shapeTemplate;
};


// ******** GroupShape *********
/// \internal
class GroupShape : public IconShape {
public:
    GroupShape(KoShapeFactory *shapeFactory) : IconShape(shapeFactory->icon()) {
        m_shapeFactory = shapeFactory;
    }

    void visit(KoCreateShapesTool *tool) {
        tool->setShapeId(m_shapeFactory->shapeId());
        tool->setShapeProperties(0);
    }

    QString toolTip() {
        return m_shapeFactory->toolTip();
    }

    const QString &groupId() const { return m_shapeFactory->shapeId(); }

private:
    KoShapeFactory *m_shapeFactory;
};


// ************** KoShapeSelector ************
KoShapeSelector::KoShapeSelector(QWidget *parent)
: QDockWidget(i18n("Shapes"), parent)
{
    setObjectName("ShapeSelector");
    m_canvas = new Canvas(this);
    setWidget(m_canvas);
    m_shapeManager = new KoShapeManager(m_canvas);
    setMinimumSize(30, 200);
    setFeatures(DockWidgetMovable|DockWidgetFloatable);

    QTimer::singleShot(0, this, SLOT(loadShapeTypes()));
}

KoShapeSelector::~KoShapeSelector() {
    delete m_shapeManager;
    delete m_canvas;
}

void KoShapeSelector::loadShapeTypes() {
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
}

void KoShapeSelector::itemSelected() {
    KoShape *koShape = m_shapeManager->selection()->firstSelectedShape();
    if(koShape == 0)
        return;
    IconShape *shape= static_cast<IconShape*> (koShape);
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController) {
        KoCreateShapesTool * tool = KoToolManager::instance()->shapeCreatorTool( canvasController->canvas() );
        shape->visit( tool );
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
}

void KoShapeSelector::add(KoShape *shape) {
    int x=5, y=5; // 5 = gap
    int w = (int) shape->size().width();
    bool ok=true; // lets be optimistic ;)
    do {
        int rowHeight=0;
        ok=true;
        foreach(const KoShape *shape, m_shapeManager->shapes()) {
            if(shape->position().y() > y || shape->position().y() + shape->size().height() < y)
                continue; // other row.
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if(x + w > width()) { // next row
                y += rowHeight + 5; // 5 = gap
                x = 5;
                ok=false;
                break;
            }
        }
    } while(! ok);
    shape->setPosition(QPointF(x, y));

    m_shapeManager->add(shape);
}


// ************ DummyViewConverter **********
QPointF KoShapeSelector::DummyViewConverter::documentToView (const QPointF &documentPoint) const {
    return documentPoint;
}

QPointF KoShapeSelector::DummyViewConverter::viewToDocument (const QPointF &viewPoint) const {
    return viewPoint;
}

QRectF KoShapeSelector::DummyViewConverter::documentToView (const QRectF &documentRect) const {
    return documentRect;
}

QRectF KoShapeSelector::DummyViewConverter::viewToDocument (const QRectF &viewRect) const {
    return viewRect;
}

void KoShapeSelector::DummyViewConverter::zoom (double *zoomX, double *zoomY) const {
    *zoomX = 1.0;
    *zoomY = 1.0;
}

double KoShapeSelector::DummyViewConverter::documentToViewX (double documentX) const {
    return documentX;
}

double KoShapeSelector::DummyViewConverter::documentToViewY (double documentY) const {
    return documentY;
}

double KoShapeSelector::DummyViewConverter::viewToDocumentX (double viewX) const {
    return viewX;
}

double KoShapeSelector::DummyViewConverter::viewToDocumentY (double viewY) const {
    return viewY;
}


// ********* Canvas **********
KoShapeSelector::Canvas::Canvas(KoShapeSelector *parent)
: QWidget(parent)
, KoCanvasBase( &m_shapeController )
, m_parent(parent)
, m_emitItemSelected(false)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setAcceptDrops(true);
}

void KoShapeSelector::Canvas::gridSize (double *horizontal, double *vertical) const {
    Q_UNUSED(horizontal);
    Q_UNUSED(vertical);
}

void KoShapeSelector::Canvas::updateCanvas (const QRectF &rc) {
    QRect rect = rc.toRect();
    rect.adjust(-2, -2, 2, 2); // grow for to anti-aliasing
    update(rect);
}

void  KoShapeSelector::Canvas::addCommand (QUndoCommand *command) {
    command->redo();
    delete command;
}

// event handlers
void KoShapeSelector::Canvas::mousePressEvent(QMouseEvent *event) {
    KoShape *clickedShape = shapeManager()->shapeAt(event->pos());
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes())
        shape->repaint();
    shapeManager()->selection()->deselectAll();
    m_emitItemSelected = false;
    if(clickedShape == 0)
        return;
    m_emitItemSelected = true;
    shapeManager()->selection()->select(clickedShape);
    clickedShape->repaint();
}

void KoShapeSelector::Canvas::mouseMoveEvent(QMouseEvent *event) {
    KoShape *clickedShape = shapeManager()->selection()->firstSelectedShape();
    if(clickedShape == 0)
        return;
    QPointF distance = clickedShape->position() - event->pos();
    if(qAbs(distance.x()) < 5 && qAbs(distance.y()) < 5)
        return;

    // start drag
    QString mimeType;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    TemplateShape *templateShape = dynamic_cast<TemplateShape*> (clickedShape);
    if(templateShape) {
        dataStream << templateShape->shapeTemplate().id;
        KoProperties *props = templateShape->shapeTemplate().properties;
        if(props)
            dataStream << props->store(); // is a QString
        else
            dataStream << QString();
        mimeType = SHAPETEMPLATE_MIMETYPE;
    }
    else {
        GroupShape *group = dynamic_cast<GroupShape*> (clickedShape);
        if(group) {
            dataStream << group->groupId();
            mimeType = SHAPEID_MIMETYPE;
        }
        else {
            kWarning() << "Unimplemented drag for this type!\n";
            return;
        }
    }
    dataStream << QPointF(event->pos() - clickedShape->position());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType, itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(static_cast<IconShape*>(clickedShape)->pixmap());
    drag->setHotSpot(event->pos() - clickedShape->position().toPoint());

    if(drag->start(Qt::CopyAction | Qt::MoveAction) != Qt::MoveAction)
        m_emitItemSelected = false;
}

void  KoShapeSelector::Canvas::dragEnterEvent(QDragEnterEvent *event) {
    if (event->source() == this && (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE) ||
                event->mimeData()->hasFormat(SHAPEID_MIMETYPE))) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void KoShapeSelector::Canvas::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if(m_emitItemSelected)
        m_parent->itemSelected();
}

void  KoShapeSelector::Canvas::dropEvent(QDropEvent *event) {
    QByteArray itemData;
    bool isTemplate = true;
    if (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE))
        itemData = event->mimeData()->data(SHAPETEMPLATE_MIMETYPE);
    else {
        isTemplate = false;
        itemData = event->mimeData()->data(SHAPEID_MIMETYPE);
    }
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QString dummy;
    dataStream >> dummy;
    if(isTemplate) {
        // a template additionally has a properties object. Lets get rid of that.
        QString properties;
        dataStream >> properties;
    }

    // and finally, there is a point.
    QPointF offset;
    dataStream >> offset;

    event->setDropAction(Qt::MoveAction);
    event->accept();
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        shape->repaint();
        shape->setPosition(event->pos() - offset);
        shape->repaint();
    }
}

void KoShapeSelector::Canvas::paintEvent(QPaintEvent * e) {
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(e->rect());

    QPen pen(Qt::blue); // TODO use the kde-wide 'selected' color.
    pen.setWidth(1);
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        painter.save();
        painter.translate(shape->position().x(), shape->position().y());
        painter.strokePath(shape->outline(), pen);
        painter.restore();
    }
    m_parent->m_shapeManager->paint( painter, *(viewConverter()), false );
    painter.end();
}

bool KoShapeSelector::Canvas::event(QEvent *e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        const QPointF pos(helpEvent->x(), helpEvent->y());
        KoShape *shape = m_parent->m_shapeManager->shapeAt(pos);
        if(shape) {
            IconShape *is = static_cast<IconShape*> (shape);
            QToolTip::showText(helpEvent->globalPos(), is->toolTip());
        }
        else
            QToolTip::showText(helpEvent->globalPos(), "");
    }
    return QWidget::event(e);
}

#include "KoShapeSelector.moc"
