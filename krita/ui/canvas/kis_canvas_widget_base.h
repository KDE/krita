/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>, (C)
 * Copyright (C) 2010 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef _KIS_CANVAS_WIDGET_BASE_
#define _KIS_CANVAS_WIDGET_BASE_

#include <QList>
#include <Qt>

#include <kis_abstract_canvas_widget.h>

class QColor;
class QImage;
class QSize;
class QContextMenuEvent;
class QKeyEvent;
class QMouseEvent;
class QTabletEvent;
class QWheelEvent;
class QInputMethodEvent;
class QVariant;

class KoViewConverter;

class KisCanvasWidgetBase : public KisAbstractCanvasWidget
{
public:
    KisCanvasWidgetBase(KisCanvas2 * canvas);

    virtual ~KisCanvasWidgetBase();

public: // KisAbstractCanvasWidget

    virtual KoToolProxy * toolProxy();

    virtual void documentOffsetMoved(const QPoint &);

    virtual QPoint documentOrigin() const;

    virtual void adjustOrigin();

    /**
     * Draw the specified decorations on the view.
     */
    virtual void drawDecorations(QPainter & gc, bool tools,
                                 const QPoint & documentOffset,
                                 const QRect & clipRect,
                                 KisCanvas2 * canvas);
    virtual void addDecoration(KisCanvasDecoration* deco);
    virtual KisCanvasDecoration* decoration(const QString& id);

protected:
    /**
     * Returns one check of the background checkerboard pattern.
     *
     * @param checkSize the size of the check
     */
    QImage checkImage(qint32 checkSize);

    /**
     * Returns the color of the border, i.e. the part of the canvas 
     * outside the image contents. 
     *
     */
    QColor borderColor() const;

    KisCanvas2 *canvas() const;

    const KoViewConverter *viewConverter() const;

    QPoint documentOffset() const;

    /// document size in widget pixels
    QSize documentSize() const;

    /// these methods take origin coordinate into account, basically it means (point - origin)
    QPoint widgetToView(const QPoint& p) const;
    QRect widgetToView(const QRect& r) const;
    QPoint viewToWidget(const QPoint& p) const;
    QRect viewToWidget(const QRect& r) const;

    /**
     * Event handlers to be called by derived canvas event handlers.
     * All common event processing is carried out by these 
     * functions.
     */
    void processKeyPressEvent(QKeyEvent *e);
    void processKeyReleaseEvent(QKeyEvent *e);
    void processMousePressEvent(QMouseEvent *e);
    void processMouseMoveEvent(QMouseEvent *e);
    void processMouseReleaseEvent(QMouseEvent *e);
    void processMouseDoubleClickEvent(QMouseEvent *e);
    void processContextMenuEvent(QContextMenuEvent *e);
    void processTabletEvent(QTabletEvent *e);
    void processWheelEvent(QWheelEvent *e);
    QVariant processInputMethodQuery(Qt::InputMethodQuery query) const;
    void processInputMethodEvent(QInputMethodEvent *event);

    /// To be implemented by the derived canvas 
    virtual void emitDocumentOriginChangedSignal() = 0;
    virtual bool callFocusNextPrevChild(bool next) = 0;

private:
    class Private;
    Private * const m_d;

};

#endif // _KIS_CANVAS_WIDGET_BASE_
