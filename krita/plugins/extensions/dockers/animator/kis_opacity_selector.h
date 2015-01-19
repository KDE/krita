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

#ifndef KIS_OPACITY_SELECTOR_H
#define KIS_OPACITY_SELECTOR_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

/**
 * The widget for seleting the opacity
 * values of the neighbouring frames for
 * onion skinning.
 */
class KisOpacitySelector : public QGraphicsItem
{
public:
    KisOpacitySelector(int x, int y, int width, int height, int type, QGraphicsScene* scene = 0, int frames = 0);
    ~KisOpacitySelector();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setOpacityValue(QList<int> l);
    QList<int>* opacityValues();
    void setOpacityValue(int frame, int val);

public:
    static const int PREV_FRAMES_OPACITY_SELECTOR = 0;
    static const int NEXT_FRAMES_OPACITY_SELECTOR = 1;

private:
    int m_width, m_height, m_x, m_y, m_frames;
    QList<int> *m_opacityValues;
};

#endif // KIS_OPACITY_SELECTOR_H
