/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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
#include "kis_timeline.h"
#include <QList>
#include "kis_animation_layer.h"
class KisLayerContents;
class KisAnimationFrame;

class KisFrameBox : public QListWidget
{
    Q_OBJECT
public:
    KisFrameBox(KisTimeline* parent = 0);
    void onCanvasReady();
    void setSelectedFrame(KisAnimationFrame* selectedFrame);
    KisAnimationFrame* getSelectedFrame();

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void updateUI();

private:
    KisTimeline* m_dock;
    QList<KisAnimationLayer*> m_layers;
    QList<KisLayerContents*> m_layerContents;
    KisAnimationFrame* m_selectedFrame;

};

#endif // KIS_FRAME_BOX_H
