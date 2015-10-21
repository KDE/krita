/* This file is part of the KDE project
 * Copyright 2014  Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CURVEEDITORITEM_H
#define CURVEEDITORITEM_H

#include "kis_cubic_curve.h"

#include <QQuickPaintedItem>

class CurveEditorItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(KisCubicCurve curve READ curve WRITE setCurve NOTIFY curveChanged);
    Q_PROPERTY(bool pointSelected READ pointSelected NOTIFY pointSelectedChanged);
public:
    explicit CurveEditorItem(QQuickItem* parent = 0);
    virtual ~CurveEditorItem();
    virtual void paint(QPainter* p);

    KisCubicCurve curve() const;
    void setCurve(KisCubicCurve curve);

    bool pointSelected() const;
    Q_SLOT void deleteSelectedPoint();

Q_SIGNALS:
    void curveChanged();
    void pointSelectedChanged();

protected:
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    class Private;
    Private* d;
};

#endif // CURVEEDITORITEM_H
