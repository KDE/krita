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

#ifndef KIS_FRAME_BOX_H
#define KIS_FRAME_BOX_H

#include <QListWidget>
#include <QList>

#include "kis_timeline.h"
#include "kis_animation_layer_widget.h"

class KisLayerContentsWidget;
class KisAnimationFrameWidget;
class KisTimelineHeader;

/**
 * This class represents the widget containing
 * all the animation frame widgets in the timeline.
 */
class KisFrameBox : public QListWidget
{
    Q_OBJECT
public:
    KisFrameBox(KisTimelineWidget* parent = 0);

    void setSelectedFrame(int x = -1, KisLayerContentsWidget* parent = 0, int width = 10);

    KisAnimationFrameWidget* getSelectedFrame();
    KisLayerContentsWidget* getFirstLayer();
    QList<KisLayerContentsWidget*> getLayerContents();

    void addLayerUiUpdate();
    void removeLayerUiUpdate(int layer);

    void moveLayerUpUiUpdate(int layer);
    void moveLayerDownUiUpdate(int layer);

    KisAnimationFrameWidget* m_selectedFrame;

private:
    KisTimelineWidget* m_dock;
    QList<KisAnimationLayerWidget*> m_layers;
    QList<KisLayerContentsWidget*> m_layerContents;
    KisTimelineHeader* m_timelineHeader;

signals:
    void frameSelectionChanged(QRect geometry);

};

#endif // KIS_FRAME_BOX_H
