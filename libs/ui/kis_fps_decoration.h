/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
