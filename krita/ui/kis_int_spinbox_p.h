/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_INT_SPINBOX_P_H
#define KIS_INT_SPINBOX_P_H

#include <QSlider>
#include <QHBoxLayout>
#include <QMenu>

class KisPopupSlider : public QMenu {

    Q_OBJECT

public:

    KisPopupSlider(int minValue, int maxValue, int pageStep, int value, Qt::Orientation orientation, QWidget * parent)
        : QMenu(parent)
    {
        m_slider = new QSlider(orientation);
        m_slider->setMinimum(minValue);
        m_slider->setMaximum(maxValue);
        m_slider->setPageStep(pageStep);
        m_slider->setValue(value);
        //m_slider->setTracking(false);
        m_slider->setMinimumSize(maxValue-minValue+20, 30);

        QHBoxLayout * l = new QHBoxLayout(this);
        l->setMargin(2);
        l->setSpacing(2);
        l->addWidget(m_slider);

        connect(m_slider, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
    }

    void setTickInterval(int i) { m_slider->setTickInterval(i); }
    void setRange(int minValue, int maxValue) { m_slider->setRange(minValue, maxValue); }
    void setValue(int val) { m_slider->setValue(val); }
    void setTickPosition(QSlider::TickPosition t) { m_slider->setTickPosition(t); }
    int singleStep () const{ return m_slider->singleStep(); }
    int minimum () const{ return m_slider->minimum(); }
    int maximum () const{ return m_slider->maximum(); }
    int value () const{ return m_slider->value(); }
    QSlider *m_slider;

signals:
    void valueChanged(int);

};

#endif
