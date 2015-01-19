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

#ifndef KIS_ANIMATION_LAYER_H
#define KIS_ANIMATION_LAYER_H

#include "kis_animation_layerbox.h"

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

/**
 * This class represents the animation layer
 * widget element in the timeline
 */
class KisAnimationLayerWidget : public QWidget
{
    Q_OBJECT

public:
    KisAnimationLayerWidget(KisAnimationLayerBox* parent = 0, int index = 1);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void onLayerNameEdited();

    void lockToggleClicked();
    void onionSkinToggleClicked();
    void visibilityToggleClicked();

private:
    KisAnimationLayerBox *m_layerBox;
    QLabel* m_lblLayerName;
    QLineEdit* m_inputLayerName;
    QPushButton* m_visibilityToggle;
    QPushButton* m_onionSkinToggle;
    QPushButton* m_lockToggle;
};

#endif // KIS_ANIMATION_LAYER_H
