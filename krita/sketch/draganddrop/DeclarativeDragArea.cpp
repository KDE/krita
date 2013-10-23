/*
    Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "DeclarativeDragArea.h"
#include "DeclarativeMimeData.h"

#include <QDrag>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>

/*!
    A DragArea is used to make an item draggable.
*/

DeclarativeDragArea::DeclarativeDragArea(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
    m_delegate(0),
    m_source(0),
    m_target(0),
    m_enabled(true),
    m_supportedActions(Qt::MoveAction),
    m_defaultAction(Qt::MoveAction),
    m_data(new DeclarativeMimeData())    // m_data is owned by us, and we shouldn't pass it to Qt directly as it will automatically delete it after the drag and drop.
{
    m_startDragDistance = QApplication::startDragDistance();
    setAcceptedMouseButtons(Qt::LeftButton);
    setFiltersChildEvents(true);
}

DeclarativeDragArea::~DeclarativeDragArea()
{
    if (m_data) {
        delete m_data;
    }
}

/*!
  The delegate is the item that will be displayed next to the mouse cursor during the drag and drop operation.
  It usually consists of a large, semi-transparent icon representing the data being dragged.
*/
QDeclarativeComponent* DeclarativeDragArea::delegate() const
{
    return m_delegate;
}

void DeclarativeDragArea::setDelegate(QDeclarativeComponent *delegate)
{
    if (m_delegate != delegate) {
        m_delegate = delegate;
        emit delegateChanged();
    }
}
void DeclarativeDragArea::resetDelegate()
{
    setDelegate(0);
}

/*!
  The QML element that is the source of this drag and drop operation. This can be defined to any item, and will
  be available to the DropArea as event.data.source
*/
QDeclarativeItem* DeclarativeDragArea::source() const
{
    return m_source;
}

void DeclarativeDragArea::setSource(QDeclarativeItem* source)
{
    if (m_source != source) {
        m_source = source;
        m_data->setSource(source);
        emit sourceChanged();
    }
}

void DeclarativeDragArea::resetSource()
{
    setSource(0);
}

// target
QDeclarativeItem* DeclarativeDragArea::target() const
{
    //TODO: implement me
    return 0;
}

// data
DeclarativeMimeData* DeclarativeDragArea::mimeData() const
{
    return m_data;
}

// startDragDistance
int DeclarativeDragArea::startDragDistance() const
{
    return m_startDragDistance;
}

void DeclarativeDragArea::setStartDragDistance(int distance)
{
    if (distance == m_startDragDistance) {
        return;
    }

    m_startDragDistance = distance;
    emit startDragDistanceChanged();
}

// delegateImage
QVariant DeclarativeDragArea::delegateImage() const
{
    return m_delegateImage;
}

void DeclarativeDragArea::setDelegateImage(const QVariant &image)
{
    if (image.canConvert<QImage>() && image.value<QImage>() == m_delegateImage) {
        return;
    }

    if (image.canConvert<QImage>()) {
        m_delegateImage = image.value<QImage>();
    } else {
        m_delegateImage = image.value<QIcon>().pixmap(QSize(48, 48)).toImage();
    }

    emit delegateImageChanged();
}

// enabled
bool DeclarativeDragArea::isEnabled() const
{
    return m_enabled;
}
void DeclarativeDragArea::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

// supported actions
Qt::DropActions DeclarativeDragArea::supportedActions() const
{
    return m_supportedActions;
}
void DeclarativeDragArea::setSupportedActions(Qt::DropActions actions)
{
    if (actions != m_supportedActions) {
        m_supportedActions = actions;
        emit supportedActionsChanged();
    }
}

// default action
Qt::DropAction DeclarativeDragArea::defaultAction() const
{
    return m_defaultAction;
}
void DeclarativeDragArea::setDefaultAction(Qt::DropAction action)
{
    if (action != m_defaultAction) {
        m_defaultAction = action;
        emit defaultActionChanged();
    }
}

void DeclarativeDragArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ( !m_enabled
         || QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length()
            < m_startDragDistance) {
        return;
    }

    emit dragStarted(event->scenePos().x(), event->scenePos().y());

    QDrag *drag = new QDrag(event->widget());
    DeclarativeMimeData* dataCopy = new DeclarativeMimeData(m_data); //Qt will take ownership of this copy and delete it.
    drag->setMimeData(dataCopy);

    if (!m_delegateImage.isNull()) {
        drag->setPixmap(QPixmap::fromImage(m_delegateImage));
    } else if (m_delegate) {
        // Render the delegate to a Pixmap
        QDeclarativeItem* item = qobject_cast<QDeclarativeItem *>(m_delegate->create());

        QGraphicsScene scene;
        scene.addItem(item);

        QPixmap pixmap(scene.sceneRect().width(), scene.sceneRect().height());
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        scene.render(&painter);
        painter.end();
        delete item;

        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(0, 0)); // TODO: Make a property for that
    }

    //setCursor(Qt::OpenHandCursor);    //TODO? Make a property for the cursor

    Qt::DropAction action = drag->exec(m_supportedActions, m_defaultAction);
    emit drop(action);
}

bool DeclarativeDragArea::sceneEventFilter(QGraphicsItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        mouseMoveEvent(me);
    }

    return QDeclarativeItem::sceneEventFilter(item, event);
}
