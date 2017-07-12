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

#ifndef DECLARATIVEDRAGAREA_H
#define DECLARATIVEDRAGAREA_H

#include <QQuickItem>
#include <QImage>

class QQmlComponent;
class DeclarativeMimeData;

class DeclarativeDragArea : public QQuickItem
{
    Q_OBJECT

    /**
     * The delegate is the item that will be displayed next to the mouse cursor during the drag and drop operation.
     * It usually consists of a large, semi-transparent icon representing the data being dragged.
     */
    Q_PROPERTY(QQuickItem* delegate READ delegate WRITE setDelegate NOTIFY delegateChanged RESET resetDelegate)

    /**
     * The QML element that is the source of the resulting drag and drop operation. This can be defined to any item, and will
     * be available to the DropArea as event.data.source
     */
    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged RESET resetSource)

    //TODO: to be implemented
    Q_PROPERTY(QQuickItem* target READ source NOTIFY targetChanged)

    /**
     * the mime data of the drag operation
     * @see DeclarativeMimeData
     */
    Q_PROPERTY(DeclarativeMimeData* mimeData READ mimeData CONSTANT)

    /**
     * If false no drag operation will be generate
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged) //TODO: Should call setAcceptDrops()

    /**
     * Supported operations, a combination of
     *  Qt.CopyAction
     *  Qt.MoveAction
     *  Qt.LinkAction
     *  Qt.ActionMask
     *  Qt.IgnoreAction
     *  Qt.TargetMoveAction
     */
    Q_PROPERTY(Qt::DropActions supportedActions READ supportedActions WRITE setSupportedActions NOTIFY supportedActionsChanged)

    /**
     * The default action will be performed during a drag when no modificators are pressed.
     */
    Q_PROPERTY(Qt::DropAction defaultAction READ defaultAction WRITE setDefaultAction NOTIFY defaultActionChanged)

    /**
     * distance in pixel after which a drag event will get started
     */
    Q_PROPERTY(int startDragDistance READ startDragDistance WRITE setStartDragDistance NOTIFY startDragDistanceChanged)

    /**
     * an image to be used as delegate. if present overrides the delegate property. in can be either a QImage or a QIcon
     */
    Q_PROPERTY(QVariant delegateImage READ delegateImage WRITE setDelegateImage NOTIFY delegateImageChanged)

public:
    DeclarativeDragArea(QQuickItem *parent=0);
    ~DeclarativeDragArea();

    QQuickItem *delegate() const;
    void setDelegate(QQuickItem* delegate);
    void resetDelegate();

    QVariant delegateImage() const;
    void setDelegateImage(const QVariant &image);
    QQuickItem* target() const;
    QQuickItem* source() const;
    void setSource(QQuickItem* source);
    void resetSource();

    bool isEnabled() const;
    void setEnabled(bool enabled);

    int startDragDistance() const;
    void setStartDragDistance(int distance);

    //supported actions
    Qt::DropActions supportedActions() const;
    void setSupportedActions(Qt::DropActions actions);

    //default action
    Qt::DropAction defaultAction() const;
    void setDefaultAction(Qt::DropAction action);

    DeclarativeMimeData* mimeData() const;

Q_SIGNALS:
    void dragStarted();
    void delegateChanged();
    void sourceChanged();
    void targetChanged();
    void dataChanged();
    void enabledChanged();
    void drop(int action);
    void supportedActionsChanged();
    void defaultActionChanged();
    void startDragDistanceChanged();
    void delegateImageChanged();

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) Q_DECL_OVERRIDE;

private:
    QQuickItem* m_delegate;
    QQuickItem* m_source;
    QQuickItem* m_target;
    bool m_enabled;
    bool m_draggingJustStarted;
    Qt::DropActions m_supportedActions;
    Qt::DropAction m_defaultAction;
    DeclarativeMimeData* const m_data;
    QImage m_delegateImage;
    int m_startDragDistance;
    QPointF m_buttonDownPos;
};

#endif // DECLARATIVEDRAGAREA_H

