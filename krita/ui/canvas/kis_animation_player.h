/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KIS_ANIMATION_PLAYER_H
#define KIS_ANIMATION_PLAYER_H

#include <QWidget>

#include "kis_qpainter_canvas.h"
#include "kis_coordinates_converter.h"

class KisCanvas2;

class KRITAIMAGE_EXPORT KisAnimationPlayer : public KisQPainterCanvas
{
    Q_OBJECT

public:
    KisAnimationPlayer(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent);
    ~KisAnimationPlayer();

    void setFramerate(float fps);
    void setRange(int firstFrame, int lastFrame);

    void play();
    void stop();
    bool isPlaying();

    void resizeEvent(QResizeEvent *e);

public slots:
    void slotUpdate();

protected:
    void drawImage(QPainter & gc, const QRect &updateWidgetRect) const;

private:
    struct Private;
    Private * const m_d;
};


#endif
