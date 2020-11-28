/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
