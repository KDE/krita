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

#ifndef KIS_OPACITY_SELECTOR_VIEW_H
#define KIS_OPACITY_SELECTOR_VIEW_H

#include "kis_opacity_selector.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QSize>
#include <QList>

/**
 * This class is the container of the opacity selector.
 */
class KisOpacitySelectorView : public QGraphicsView
{
    Q_OBJECT
public:
    KisOpacitySelectorView(QWidget* parent, int type);
    ~KisOpacitySelectorView();

    QSize sizeHint() const;

    KisOpacitySelector* opacitySelector();
    int numberOfFrames();

private:
    void init();

private:
    QGraphicsScene *m_opacitySelectorScene;
    KisOpacitySelector* m_opacitySelector;
    int m_numberOfFrames;
    int m_type;

protected:
    void mousePressEvent(QMouseEvent *event);

public slots:
    void setNumberOfFrames(int val);

signals:
    void opacityValueChanged();
};

#endif // KIS_OPACITY_SELECTOR_VIEW_H
