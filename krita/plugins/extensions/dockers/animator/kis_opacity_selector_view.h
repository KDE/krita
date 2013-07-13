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

#ifndef KIS_OPACITY_SELECTOR_VIEW_H
#define KIS_OPACITY_SELECTOR_VIEW_H
#include "kis_opacity_selector.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QSize>
#include <QList>

class KisOpacitySelectorView : public QGraphicsView
{
    Q_OBJECT
public:
    KisOpacitySelectorView(QWidget* parent);
    ~KisOpacitySelectorView();
    QGraphicsScene *m_opacitySelectorScene;
    KisOpacitySelector* m_opacitySelector;
    QSize sizeHint() const;
    void init();

private:
    int m_numberOfFrames;

protected:
    void mousePressEvent(QMouseEvent *event);

public slots:
    void setNumberOfFrames(int val);

};

#endif // KIS_OPACITY_SELECTOR_VIEW_H
