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
class QPointF;
class QTabletEvent;
class QWheelEvent;
class QInputMethodEvent;
class QVariant;

class KoViewConverter;
class KisCoordinatesConverter;

class KisCanvasWidgetBase : public KisAbstractCanvasWidget
{
public:
    KisCanvasWidgetBase(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter);

    virtual ~KisCanvasWidgetBase();

public: // KisAbstractCanvasWidget

    virtual KoToolProxy * toolProxy();

    /**
     * Draw the specified decorations on the view.
     */
    virtual void drawDecorations(QPainter & gc, const QRect &updateWidgetRect);

    virtual void addDecoration(KisCanvasDecoration* deco);
    virtual KisCanvasDecoration* decoration(const QString& id);

    virtual void setDecorations(const QList<KisCanvasDecoration*> &);
    virtual QList<KisCanvasDecoration*> decorations();

    /**
     * Returns the color of the border, i.e. the part of the canvas
     * outside the image contents.
     *
     */
    QColor borderColor() const;

    /**
     * Returns one check of the background checkerboard pattern.
     */
    static QImage checkImage(qint32 checkSize = -1);

protected:
    KisCanvas2 *canvas() const;

    KisCoordinatesConverter* coordinatesConverter();

    /**
     * Event handlers to be called by derived canvas event handlers.
     * All common event processing is carried out by these
     * functions.
     */
    QVariant processInputMethodQuery(Qt::InputMethodQuery query) const;
    void processInputMethodEvent(QInputMethodEvent *event);
    void notifyConfigChanged();

    /// To be implemented by the derived canvas
    virtual bool callFocusNextPrevChild(bool next) = 0;

private:
    struct Private;
    Private * const m_d;

};

#endif // _KIS_CANVAS_WIDGET_BASE_
