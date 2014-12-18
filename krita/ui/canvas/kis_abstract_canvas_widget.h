/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>, (C)
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
#ifndef _KIS_ABSTRACT_CANVAS_WIDGET_
#define _KIS_ABSTRACT_CANVAS_WIDGET_

class QWidget;
class QRect;
class QPoint;
class QPainter;
class QRect;

class KoToolProxy;

class KisCanvas2;
class KisCanvasDecoration;
class KisDisplayFilter;

#include "kis_types.h"


class KisAbstractCanvasWidget
{

public:

    KisAbstractCanvasWidget() {}

    virtual ~KisAbstractCanvasWidget() {}

    virtual QWidget * widget() = 0;

    virtual KoToolProxy * toolProxy() const = 0;

    /**
     * Draw the specified decorations on the view.
     */
    virtual void drawDecorations(QPainter & gc, const QRect &updateWidgetRect) const = 0;

    virtual void addDecoration(KisCanvasDecoration* deco) = 0;
    virtual KisCanvasDecoration* decoration(const QString& id) const = 0;

    virtual void setDecorations(const QList<KisCanvasDecoration*> &) = 0;
    virtual QList<KisCanvasDecoration*> decorations() const = 0;

    /// set the specified display filter on the canvas
    virtual void setDisplayFilter(KisDisplayFilter *displayFilter) = 0;

    virtual void setWrapAroundViewingMode(bool value) = 0;

    /**
     * Returns true if the asynchromous engine of the canvas
     * (e.g. openGL pipeline) is busy with processing of the previous
     * update events. This will make KisCanvas2 to postpone and
     * compress update events.
     */
    virtual bool isBusy() const = 0;
};

#endif // _KIS_ABSTRACT_CANVAS_WIDGET_
