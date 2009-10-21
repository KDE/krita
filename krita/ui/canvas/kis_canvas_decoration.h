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

class QPoint;
class QRect;
class QPainter;
class KoViewConverter;
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
     * @param documentOffset the offset of the view in the document, expressed in the view reference (not in the document reference)
     */
    void paint(QPainter& gc, const QPoint & documentOffset, const QRect& area, const KoViewConverter &converter);
public slots:
    /**
     * Set if the decoration is visible or not.
     */
    void setVisible(bool v);
    /**
     * If decoration is visible, hide it, if not show it.
     */
    void toggleVisibility();
protected:
    virtual void drawDecoration(QPainter& gc, const QPoint & documentOffset, const QRect& area, const KoViewConverter &converter) = 0;
    /**
     * @return the parent KisView
     */
    KisView2* view() const;
private:
    struct Private;
    Private* const d;
};

#endif
