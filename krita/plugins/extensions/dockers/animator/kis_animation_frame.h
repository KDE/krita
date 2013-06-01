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

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>

class KisAnimationFrame : public QGraphicsItem
{
public:
    KisAnimationFrame(int x, int y, int width, int height, int type);
    ~KisAnimationFrame();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int getFrameType();
    int getY();
    int getX();
    KisAnimationFrame* getNextFrame();
    KisAnimationFrame* getPreviousFrame();
    void setNextFrame(KisAnimationFrame* nextFrame);
    void setPreviousFrame(KisAnimationFrame* previousFrame);

private:
    int m_x, m_y, m_width, m_height;
    int m_type;
    KisAnimationFrame* m_nextFrame;
    KisAnimationFrame* m_previousFrame;

public:
    const static int BLANKFRAME = 1;
    const static int KEYFRAME = 2;
    const static int SELECTEDFRAME = 0;
    const static int CONTINUEFRAME = 3;
};

#endif // KIS_ANIMATION_FRAME_H
