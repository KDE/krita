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

#ifndef KIS_FRAME_BOX_H
#define KIS_FRAME_BOX_H

#include <QListWidget>
#include <QList>

#include "kis_timeline.h"
#include "kis_animation_layer.h"

class KisLayerContents;
class KisAnimationFrame;
class KisTimelineHeader;

class KisFrameBox : public QListWidget
{
    Q_OBJECT
public:
    KisFrameBox(KisTimeline* parent = 0);

    void setSelectedFrame(int x=-1, KisLayerContents* parent=0, int width=10);

    KisAnimationFrame* getSelectedFrame();
    KisLayerContents* getFirstLayer();
    QList<KisLayerContents*> getLayerContents();

    void addLayerUiUpdate();
    void removeLayerUiUpdate(int layer);

    void moveLayerUpUiUpdate(int layer);
    void moveLayerDownUiUpdate(int layer);

    KisAnimationFrame* m_selectedFrame;

private:
    KisTimeline* m_dock;
    QList<KisAnimationLayer*> m_layers;
    QList<KisLayerContents*> m_layerContents;
    KisTimelineHeader* m_timelineHeader;

signals:
    void frameSelectionChanged(QRect geometry);

};

#endif // KIS_FRAME_BOX_H
