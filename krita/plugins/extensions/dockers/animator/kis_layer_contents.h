/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_LAYER_CONTENTS_H
#define KIS_LAYER_CONTENTS_H

#include "kis_frame_box.h"

#include <QWidget>
#include <QHash>

class KisAnimationFrameWidget;

/**
 * This is a widget class representing
 * the contents of an animation layer.
 */
class KisLayerContentsWidget : public QWidget
{
    Q_OBJECT

public:
    KisLayerContentsWidget(KisFrameBox* parent = 0);

    void mapFrame(int frameNumber, KisAnimationFrameWidget* frame);
    void unmapFrame(int frameNumber);

    int getLastFrameIndex();
    int getPreviousFrameIndexFrom(int index);
    int getNextFrameIndexFrom(int index);
    int getIndex(KisAnimationFrameWidget* frame);

    KisAnimationFrameWidget* getNextFrameFrom(KisAnimationFrameWidget* frame);
    KisAnimationFrameWidget* getNextFrameFrom(int index);
    KisAnimationFrameWidget* getPreviousFrameFrom(KisAnimationFrameWidget* frame);
    KisAnimationFrameWidget* getPreviousFrameFrom(int index);
    KisAnimationFrameWidget* getFrameAt(int index);

    int getContentLength();
    KisFrameBox* getParent();
    int getLayerIndex();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void initialize();

private:
    KisFrameBox* m_parent;
    QHash<int, KisAnimationFrameWidget*> m_frames;
};

#endif // KIS_LAYER_CONTENTS_H
