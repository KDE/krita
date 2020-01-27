/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_ZOOM_BUTTON_H
#define _KIS_ZOOM_BUTTON_H

#include "kis_draggable_tool_button.h"

class KisZoomButton : public KisDraggableToolButton
{
    Q_OBJECT
public:
    KisZoomButton(QWidget *parent);
    ~KisZoomButton() override;

    qreal zoomLevel() const;
    void setZoomLevel(qreal level);

    void beginZoom(QPoint mousePos, qreal staticPoint);
    void continueZoom(QPoint mousePos);

    void mousePressEvent(QMouseEvent *e) override;

Q_SIGNALS:
    void zoomStarted(qreal staticPoint);
    void zoomLevelChanged(qreal level);

private Q_SLOTS:
    void slotValueChanged(int value);

private:
    qreal m_zoomLevel {1.0};
    qreal m_initialDragZoomLevel {1.0};
};

#endif
