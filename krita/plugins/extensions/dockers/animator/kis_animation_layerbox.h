/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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

#include <QWidget>
#include <QPointer>
#include <kis_types.h>
#include <QToolButton>
#include <QList>
#include <QListWidget>

class KisNodeModel;
class KisNodeManager;
class KisTimeline;
class KisAnimationLayer;

class KisAnimationLayerBox : public QListWidget
{
    Q_OBJECT
public:
    KisAnimationLayerBox(KisTimeline* parent = 0);
    void onCanvasReady();
    QList<KisAnimationLayer*> getLayers();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    KisTimeline* m_dock;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    QList<KisAnimationLayer*> m_layers;

private slots:
    void updateUI();

private:
    inline void connectActionToButton(QAction *button, const QString &id);
};

#endif // KIS_ANIMATION_LAYERBOX_H
