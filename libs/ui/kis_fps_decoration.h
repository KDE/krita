/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FPS_DECORATION_H
#define __KIS_FPS_DECORATION_H

#include "canvas/kis_canvas_decoration.h"

#include <QFont>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsDropShadowEffect;

class KisFpsDecoration : public KisCanvasDecoration
{
public:
    KisFpsDecoration(QPointer<KisView> view);
    ~KisFpsDecoration() override;

    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas) override;
    static const QString idTag;

private:
    bool draw(const QString &text, QSize &outSize);
	QString getText() const;

    QFont m_font;
    QPixmap m_pixmap;

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsDropShadowEffect *m_shadow;
};

#endif /* __KIS_FPS_DECORATION_H */
