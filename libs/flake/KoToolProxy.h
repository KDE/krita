/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _KO_TOOL_PROXY_H_
#define _KO_TOOL_PROXY_H_

#include "kritaflake_export.h"

#include <QObject>
#include <QHash>

class QAction;
class QAction;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QTabletEvent;
class KoCanvasBase;
class KoViewConverter;
class KoToolBase;
class KoToolProxyPrivate;
class QInputMethodEvent;
class KoPointerEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTouchEvent;
class QPainter;
class QPointF;
class QMenu;

/**
 * Tool proxy object which allows an application to address the current tool.
 *
 * Applications typically have a canvas and a canvas requires a tool for
 * the user to do anything.  Since the flake system is responsible for handling
 * tools and also to change the active tool when needed we provide one class
 * that can be used by an application canvas to route all the native events too
 * which will transparently be routed to the active tool.  Without the application
 * having to bother about which tool is active.
 */
class KRITAFLAKE_EXPORT KoToolProxy : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param canvas Each canvas has 1 toolProxy. Pass the parent here.
     * @param parent a parent QObject for memory management purposes.
     */
    explicit KoToolProxy(KoCanvasBase *canvas, QObject *parent = 0);
    ~KoToolProxy() override;

    /// Forwarded to the current KoToolBase
    void paint(QPainter &painter, const KoViewConverter &converter);

    /// Forwarded to the current KoToolBase
    void repaintDecorations();

    /// Forwarded to the current KoToolBase
    void tabletEvent(QTabletEvent *event, const QPointF &point);

    /// Forwarded to the current KoToolBase
    void mousePressEvent(QMouseEvent *event, const QPointF &point);
    void mousePressEvent(KoPointerEvent *event);

    /// Forwarded to the current KoToolBase
    void mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point);
    void mouseDoubleClickEvent(KoPointerEvent *event);

    /// Forwarded to the current KoToolBase
    void mouseMoveEvent(QMouseEvent *event, const QPointF &point);
    void mouseMoveEvent(KoPointerEvent *event);

    /// Forwarded to the current KoToolBase
    void mouseReleaseEvent(QMouseEvent *event, const QPointF &point);
    void mouseReleaseEvent(KoPointerEvent *event);

    /// Forwarded to the current KoToolBase
    void keyPressEvent(QKeyEvent *event);

    /// Forwarded to the current KoToolBase
    void keyReleaseEvent(QKeyEvent *event);

    /// Forwarded to the current KoToolBase
    void explicitUserStrokeEndRequest();

    /// Forwarded to the current KoToolBase
    QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;

    /// Forwarded to the current KoToolBase
    void inputMethodEvent(QInputMethodEvent *event);

    /// Forwarded to the current KoToolBase
    QMenu* popupActionsMenu();

    /// Forwarded to the current KoToolBase
    QWidget* popupWidget();

    /// Forwarded to the current KoToolBase
    void deleteSelection();

    /// This method gives the proxy a chance to do things. for example it is need to have working singlekey
    /// shortcuts. call it from the canvas' event function and forward it to QWidget::event() later.
    void processEvent(QEvent *) const;

    /// returns true if the current tool holds a selection
    bool hasSelection() const;

    /// Forwarded to the current KoToolBase
    void cut();

    /// Forwarded to the current KoToolBase
    void copy() const;

    /// Forwarded to the current KoToolBase
    bool paste();

    /// Forwarded to the current KoToolBase
    void dragMoveEvent(QDragMoveEvent *event, const QPointF &point);

    /// Forwarded to the current KoToolBase
    void dragLeaveEvent(QDragLeaveEvent *event);

    /// Forwarded to the current KoToolBase
    void dropEvent(QDropEvent *event, const QPointF &point);

    /// Set the new active tool.
    virtual void setActiveTool(KoToolBase *tool);

    void touchEvent(QTouchEvent* event, const QPointF& point);

    /// \internal
    KoToolProxyPrivate *priv();

protected Q_SLOTS:
    /// Forwarded to the current KoToolBase
    void requestUndoDuringStroke();

    /// Forwarded to the current KoToolBase
    void requestRedoDuringStroke();

    /// Forwarded to the current KoToolBase
    void requestStrokeCancellation();

    /// Forwarded to the current KoToolBase
    void requestStrokeEnd();

Q_SIGNALS:
    /**
     * A tool can have a selection that is copy-able, this signal is emitted when that status changes.
     * @param hasSelection is true when the tool holds selected data.
     */
    void selectionChanged(bool hasSelection);

    /**
     * Emitted every time a tool is changed.
     * @param toolId the id of the tool.
     * @see KoToolBase::toolId()
     */
    void toolChanged(const QString &toolId);

protected:
    virtual QPointF widgetToDocument(const QPointF &widgetPoint) const;
    KoCanvasBase* canvas() const;

private:
    Q_PRIVATE_SLOT(d, void timeout())
    Q_PRIVATE_SLOT(d, void selectionChanged(bool))

    friend class KoToolProxyPrivate;
    KoToolProxyPrivate * const d;
};

#endif // _KO_TOOL_PROXY_H_
