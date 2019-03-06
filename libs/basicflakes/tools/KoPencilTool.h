/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KOPENCILTOOL_H_
#define _KOPENCILTOOL_H_

#include "KoFlakeTypes.h"
#include "KoToolBase.h"

class KoPathShape;
class KoShapeStroke;
class KoPathPoint;
class KoStrokeConfigWidget;

#include "kritabasicflakes_export.h"

class KRITABASICFLAKES_EXPORT KoPencilTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KoPencilTool(KoCanvasBase *canvas);
    ~KoPencilTool() override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void repaintDecorations() override;

    void mousePressEvent(KoPointerEvent *event) override ;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void activate(ToolActivation activation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

protected:
    QList<QPointer<QWidget> > createOptionWidgets() override;

    /**
     * Add path shape to document.
     * This method can be overridden and change the behaviour of the tool. In that case the subclass takes ownership of pathShape.
     * It gets only called, if there are two or more points in the path.
     */
    virtual void addPathShape(KoPathShape* path, bool closePath);

    KoShapeStrokeSP createStroke();
    KoPathShape * path();
    void setFittingError(qreal fittingError);
    qreal getFittingError();

private Q_SLOTS:
    void selectMode(int mode);
    void setOptimize(int state);
    void setDelta(double delta);

protected Q_SLOTS:
    virtual void slotUpdatePencilCursor();

private:

    qreal lineAngle(const QPointF &p1, const QPointF &p2);
    void addPoint(const QPointF & point);
    void finish(bool closePath);

    /// returns the nearest existing path point
    KoPathPoint* endPointAtPosition(const QPointF &position);

    /// Connects given path with the ones we hit when starting/finishing
    bool connectPaths(KoPathShape *pathShape, KoPathPoint *pointAtStart, KoPathPoint *pointAtEnd);

    enum PencilMode { ModeRaw, ModeCurve, ModeStraight };

    PencilMode m_mode;
    bool m_optimizeRaw;
    bool m_optimizeCurve;
    qreal m_combineAngle;
    qreal m_fittingError;
    bool m_close;

    QList<QPointF> m_points; // the raw points

    KoPathShape * m_shape;
    KoPathPoint *m_existingStartPoint; ///< an existing path point we started a new path at
    KoPathPoint *m_existingEndPoint;   ///< an existing path point we finished a new path at
    KoPathPoint *m_hoveredPoint; ///< an existing path end point the mouse is hovering on
    KoStrokeConfigWidget *m_strokeWidget;
};

#endif // _KOPENCILTOOL_H_
