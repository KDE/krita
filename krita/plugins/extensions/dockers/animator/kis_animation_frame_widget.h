/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ANIMATION_FRAME_H
#define KIS_ANIMATION_FRAME_H

#include <QWidget>

#include "kis_layer_contents.h"

/**
 * This class represents the animation frame
 * widget element in the timeline.
 */
class KisAnimationFrameWidget : public QWidget
{
    Q_OBJECT

public:
    KisAnimationFrameWidget(KisLayerContentsWidget* parent = 0, int type = 0, int width = 10);

    void setWidth(int width);
    int getWidth();

    KisLayerContentsWidget* getParent();

    QRect convertSelectionToFrame();

    int getType();
    void setType(int type);

    void expandWidth();

    QRect removeFrame();

    int getIndex();

protected:
    void paintEvent(QPaintEvent *event);

public:
    static const int SELECTION = 0;
    static const int FRAME = 1;

private:
    int m_type;
    int m_width;
    KisLayerContentsWidget* m_parent;
};

#endif // KIS_ANIMATION_FRAME_H
