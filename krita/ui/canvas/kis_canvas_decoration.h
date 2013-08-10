/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_CANVAS_DECORATION_H_
#define _KIS_CANVAS_DECORATION_H_

#include <QObject>
#include <krita_export.h>
#include <kis_canvas2.h>

class QPoint;
class QRect;
class QRectF;
class QPainter;
class KoViewConverter;
class KisCoordinatesConverter;
class KisView2;

/**
 * This class is the base class for object that draw a decoration on the canvas,
 * for instance, selections, grids, tools, ...
 */
class KRITAUI_EXPORT KisCanvasDecoration : public QObject
{
    Q_OBJECT
public:
    KisCanvasDecoration(const QString& id, const QString& name, KisView2 * parent);
    ~KisCanvasDecoration();
    const QString& id() const;
    const QString& name() const;
    /**
     * @return whether the decoration is visible.
     */
    bool visible() const;

    /**
     * Will paint the decoration on the QPainter, if the visible is set to true.
     *
     * @param updateRect dirty rect in document pixels
     */
    void paint(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas);

public slots:
    /**
     * Set if the decoration is visible or not.
     */
    virtual void setVisible(bool v);
    /**
     * If decoration is visible, hide it, if not show it.
     */
    virtual void toggleVisibility();
protected:
    virtual void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter,KisCanvas2* canvas) = 0;

    /**
     * @return the parent KisView
     */
    KisView2* view() const;
private:
    struct Private;
    Private* const d;
};

#endif
