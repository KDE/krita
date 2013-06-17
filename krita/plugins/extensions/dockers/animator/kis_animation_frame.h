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

#ifndef KIS_ANIMATION_FRAME_H
#define KIS_ANIMATION_FRAME_H

#include <QWidget>
#include <kis_layer_contents.h>

class KisAnimationFrame : public QWidget
{
    Q_OBJECT

public:
    KisAnimationFrame(KisLayerContents* parent = 0, int type = 0);

protected:
    void paintEvent(QPaintEvent *event);

public:
    static const int SELECTION = 0;
    static const int KEYFRAME = 1;
    static const int BLANKFRAME = 2;
};

#endif // KIS_ANIMATION_FRAME_H
