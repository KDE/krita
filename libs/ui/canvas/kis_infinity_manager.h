/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
