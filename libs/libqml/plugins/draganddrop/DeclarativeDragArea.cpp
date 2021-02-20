/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
*/

#include "DeclarativeDragArea.h"
#include "DeclarativeMimeData.h"

#include <QDrag>
#include <QIcon>
#include <QGuiApplication>
#include <QStyleHints>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QQmlContext>
#include <QQuickWindow>

#include <QDebug>

#include <kis_icon_utils.h>

/*!
    A DragArea is used to make an item draggable.
*/

DeclarativeDragArea::DeclarativeDragArea(QQuickItem *parent)
    : QQuickItem(parent),
    m_delegate(0),
    m_source(parent),
    m_target(0),
    m_enabled(true),
    m_draggingJustStarted(false),
    m_supportedActions(Qt::MoveAction),
    m_defaultAction(Qt::MoveAction),
    m_data(new DeclarativeMimeData())    // m_data is owned by us, and we shouldn't pass it to Qt directly as it will automatically delete it after the drag and drop.
{
    m_startDragDistance = QGuiApplication::styleHints()->startDragDistance();
    setAcceptedMouseButtons(Qt::LeftButton);
//     setFiltersChildEvents(true);
    setFlag(ItemAcceptsDrops, m_enabled);
    setAcceptHoverEvents(true);
    setFiltersChildMouseEvents(true);
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
QQuickItem* DeclarativeDragArea::delegate() const
{
    return m_delegate;
}

void DeclarativeDragArea::setDelegate(QQuickItem *delegate)
{
    if (m_delegate != delegate) {
        //qDebug() << " ______________________________________________ " << delegate;
        m_delegate = delegate;
        emit delegateChanged();
    }
}
void DeclarativeDragArea::resetDelegate()
{
    setDelegate(0);
}

/*!
  The QML element that is the source of this drag and drop operation. This can be
  defined to any item, and will be available to the DropArea as event.data.source
*/
QQuickItem* DeclarativeDragArea::source() const
{
    return m_source;
}

void DeclarativeDragArea::setSource(QQuickItem* source)
{
    if (m_source != source) {
        m_source = source;
        emit sourceChanged();
    }
}

void DeclarativeDragArea::resetSource()
{
    setSource(0);
}

// target
QQuickItem* DeclarativeDragArea::target() const
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

void DeclarativeDragArea::mousePressEvent(QMouseEvent* event)
{
    m_buttonDownPos = event->screenPos();
    m_draggingJustStarted = true;
    setKeepMouseGrab(true);
}

void DeclarativeDragArea::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_draggingJustStarted = false;
    setKeepMouseGrab(false);
    ungrabMouse();
}

void DeclarativeDragArea::mouseMoveEvent(QMouseEvent *event)
{
    if ( !m_enabled
         || QLineF(event->screenPos(), m_buttonDownPos).length()
            < m_startDragDistance) {
        return;
    }

    if (m_draggingJustStarted) {
        grabMouse();
        m_draggingJustStarted = false;
        //qDebug() << "************ DDDD new QDrag" << objectName();
        QDrag *drag = new QDrag(parent());
        DeclarativeMimeData* dataCopy = new DeclarativeMimeData(m_data); //Qt will take ownership of this copy and delete it.
        dataCopy->setText(objectName());
        drag->setMimeData(dataCopy);
        //qDebug() << "-----> data: dragarea: " << this << x() << y() << " mimedata: " << m_data << (dataCopy->hasColor() ? dataCopy->color().name() : " no color") ;

        const QSize _s(48,48); // FIXME: smarter, please

        if (!m_delegateImage.isNull()) {
            drag->setPixmap(QPixmap::fromImage(m_delegateImage));
//             qDebug() << "++++++delegateImage";
        } else {
//             qDebug() << "DDD NO Delegate image";
            if (m_delegate) {
                // This is just highly unreliable, let's completely skip this
                // until we have a non-disgusting way of "attaching an item to
                // the cursor
//                 QRectF rf;
//                 qDebug() << "DDD +++ delegate" << m_delegate;
//                 rf = QRectF(0, 0, m_delegate->width(), m_delegate->height());
//                 rf = m_delegate->mapRectToScene(rf);
//                 QImage grabbed = window()->grabWindow();
//                 //rf = rf.intersected(QRectF(0, 0, grabbed.width(), grabbed.height()));
//                 grabbed = grabbed.copy(rf.toAlignedRect());
// //                 qDebug() << " +++++++++++++++++++++++ dim: " << rf;
//                 grabbed.save("file:///tmp/grabbed.png");
//                 QPixmap pm = QPixmap::fromImage(grabbed);
//                 qDebug() << " set new pixmap" << grabbed.size();
//                 drag->setPixmap(pm);

            } else if (mimeData()->hasImage()) {
//                 qDebug() << "++++++hasImage";
                QImage im = qvariant_cast<QImage>(mimeData()->imageData());
                drag->setPixmap(QPixmap::fromImage(im));
            } else if (mimeData()->hasColor()) {
//                 qDebug() << "++++++color";
                QPixmap px(_s);
                px.fill(mimeData()->color());
                drag->setPixmap(px);
            } else {
                // icons otherwise
                QStringList icons;
//                 qDebug() << "DDD adding icons dan maar";
                if (mimeData()->hasText()) {
                    icons << "text-plain";
                }
                if (mimeData()->hasHtml()) {
                    icons << "text-html";
                }
                if (mimeData()->hasUrls()) {
                    Q_FOREACH (const QVariant &u, mimeData()->urls()) {
                        Q_UNUSED(u);
                        icons << "text-html";
                    }
                }
                if (icons.count()) {
                    const int _w = 48;
                    QPixmap pm(_w*icons.count(), _w);
                    pm.fill(Qt::transparent);
                    QPainter p(&pm);
                    int i = 0;
                    Q_FOREACH (const QString &ic, icons) {
                        p.drawPixmap(QPoint(i*_w, 0), KisIconUtils::loadIcon(ic).pixmap(_w, _w));
                        i++;
                    }
                    p.end();
                    drag->setPixmap(pm);
                }
                //qDebug() << "DD pixmaps set for icons: " << icons;
            }

        }

        //drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2)); // TODO: Make a property for that
        //setCursor(Qt::OpenHandCursor);    //TODO? Make a property for the cursor

        emit dragStarted();

        Qt::DropAction action = drag->exec(m_supportedActions, m_defaultAction);
        setKeepMouseGrab(false);
        emit drop(action);
        ungrabMouse();
    }
}

bool DeclarativeDragArea::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        //qDebug() << "press in dragarea";
        mousePressEvent(me);
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        //qDebug() << "move in dragarea";
        mouseMoveEvent(me);
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        //qDebug() << "release in dragarea";
        mouseReleaseEvent(me);
        break;
    }
    default:
        break;
    }

    return QQuickItem::childMouseEventFilter(item, event);
}

