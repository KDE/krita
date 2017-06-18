/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_INFINITY_MANAGER_H
#define __KIS_INFINITY_MANAGER_H

#include "kis_canvas_decoration.h"

#include <QPointer>
#include <QPainterPath>
#include <QCursor>

#include <kis_canvas2.h>

class KisView;

static const QString INFINITY_DECORATION_ID = "infinity-decorations";

class KRITAUI_EXPORT KisInfinityManager : public KisCanvasDecoration
{
Q_OBJECT

public:
KisInfinityManager(QPointer<KisView>view, KisCanvas2 *canvas);

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter, KisCanvas2 *canvas) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

public Q_SLOTS:
    void imagePositionChanged();

private:
    enum Side {
        Right = 0,
        Bottom,
        Left,
        Top,

        NSides
    };
    inline void addDecoration(const QRect &areaRect, const QPointF &handlePoint, qreal angle, Side side);

private:
    QPainterPath m_decorationPath;

    bool m_filteringEnabled;
    bool m_cursorSwitched;
    QCursor m_oldCursor;
    QVector<QTransform> m_handleTransform;

    QVector<QRect> m_sideRects;

    QPointer<KisCanvas2> m_canvas;
};

#endif /* __KIS_INFINITY_MANAGER_H */
