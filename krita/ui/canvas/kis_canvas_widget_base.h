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

#include <kis_abstract_canvas_widget.h>

class QColor;
class QImage;

class KisCanvasWidgetBase : public KisAbstractCanvasWidget
{

public:

    KisCanvasWidgetBase();

    virtual ~KisCanvasWidgetBase();

    //virtual KoToolProxy * toolProxy();

    //virtual void documentOffsetMoved(const QPoint &);

    //virtual QPoint documentOrigin();

    //virtual void adjustOrigin();

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

private:
    QList<KisCanvasDecoration*> m_decorations;
};

#endif // _KIS_CANVAS_WIDGET_BASE_
