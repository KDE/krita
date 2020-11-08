/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
