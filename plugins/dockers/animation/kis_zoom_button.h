/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
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

Q_SIGNALS:
    void zoom(qreal delta);

private Q_SLOTS:
    void slotValueChanged(int value);

};

#endif
