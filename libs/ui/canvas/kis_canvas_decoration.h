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
#include <QPointer>

#include <kritaui_export.h>
#include <kis_image.h>
#include "KisView.h"
#include <kis_shared.h>

class KisCanvas2;
class QRectF;
class QPainter;
class KisCoordinatesConverter;

class KisCanvasDecoration;
typedef KisSharedPtr<KisCanvasDecoration> KisCanvasDecorationSP;

/**
 * This class is the base class for object that draw a decoration on the canvas,
 * for instance, selections, grids, tools, ...
 */
class KRITAUI_EXPORT KisCanvasDecoration : public QObject, public KisShared
{
    Q_OBJECT
public:
    KisCanvasDecoration(const QString& id, QPointer<KisView>parent);

    ~KisCanvasDecoration() override;

    void setView(QPointer<KisView> imageView);

    const QString& id() const;

    /**
     * @return whether the decoration is visible.
     */
    bool visible() const;

    /**
     * Will paint the decoration on the QPainter, if the visible is set to @c true.
     *
     * @param gc the painter
     * @param updateRect dirty rect in document pixels
     * @param converter coordinate converter
     * @param canvas the canvas
     */
    void paint(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas);

    /**
     * Return z-order priority of the decoration. The higher the priority, the higher
     * the decoration is painted.
     */
    int priority() const;

    static bool comparePriority(KisCanvasDecorationSP decoration1, KisCanvasDecorationSP decoration2);

public Q_SLOTS:
    /**
     * Set if the decoration is visible or not.
     */
    virtual void setVisible(bool v);
    /**
     * If decoration is visible, hide it, if not show it.
     */
    void toggleVisibility();
protected:
    virtual void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter,KisCanvas2* canvas) = 0;

    /// XXX: unify view and imageview!
    QPointer<KisView>imageView();

    /**
     * @return the parent KisView
     */
    QPointer<KisView> view() const;

    /**
     * Set the priority of the decoration. The higher the priority, the higher
     * the decoration is painted.
     */
    void setPriority(int value);

private:
    struct Private;
    Private* const d;
};

#endif
