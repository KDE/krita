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

#ifndef KIS_ANIMATION_LAYERBOX_H
#define KIS_ANIMATION_LAYERBOX_H

#include "kis_types.h"
#include "kis_canvas2.h"

#include <QWidget>
#include <QPointer>
#include <QToolButton>
#include <QList>
#include <QListWidget>

class KisNodeModel;
class KisNodeManager;
class KisTimelineWidget;
class KisAnimationLayerWidget;

/**
 * This class represents the widget containing
 * all the animation layer widgets.
 */
class KisAnimationLayerBox : public QListWidget
{
    Q_OBJECT
public:
    KisAnimationLayerBox(KisTimelineWidget* parent = 0);

    QList<KisAnimationLayerWidget*> getLayers();

    void addLayerUiUpdate();
    void removeLayerUiUpdate(int layer);

    void moveLayerUpUiUpdate(int layer);
    void moveLayerDownUiUpdate(int layer);

    int numberOfLayers();

    void setOnionSkinState(int layer, bool state);
    bool onionSkinstate(int layer);
    QHash<int, bool> onionSkinStates();

    void setVisibilityState(int layer, bool state);
    bool visibilityState(int layer);
    QHash<int, bool> visibilityStates();

    void setLockState(int layer, bool state);
    bool lockState(int layer);
    QHash<int, bool> lockStates();

    int indexOf(KisAnimationLayerWidget* layer);

    KisCanvas2* getCanvas();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    KisTimelineWidget* m_dock;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    QList<KisAnimationLayerWidget*> m_layers;
    int m_layerIndex;

    QHash<int, bool> m_onionSkinStates;
    QHash<int, bool> m_visibilityStates;
    QHash<int, bool> m_lockStates;
};

#endif // KIS_ANIMATION_LAYERBOX_H
