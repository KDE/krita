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

#ifndef KIS_ANIMATION_LAYER_H
#define KIS_ANIMATION_LAYER_H

#include "kis_animation_layerbox.h"
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

class KisAnimationLayer : public QWidget
{
    Q_OBJECT

public:
    KisAnimationLayer(KisAnimationLayerBox* parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void onLayerNameEdited();

private:
    KisAnimationLayerBox *m_layerBox;
    QLabel* m_lblLayerName;
    QLineEdit* m_inputLayerName;
    QHBoxLayout* lay;
};

#endif // KIS_ANIMATION_LAYER_H
