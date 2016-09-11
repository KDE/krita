/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef _KIS_STOP_GRADIENT_SLIDER_WIDGET_H_
#define _KIS_STOP_GRADIENT_SLIDER_WIDGET_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

#include <resources/KoStopGradient.h>

class KisStopGradientSliderWidget : public QWidget
{
    Q_OBJECT

public:
    KisStopGradientSliderWidget(QWidget *parent = 0, const char* name = 0, Qt::WFlags f = 0);

public:
    virtual void paintEvent(QPaintEvent *);
    void setGradientResource(KoStopGradient* gradient);

    int selectedStop();
    
    void setSeletectStop(int selected);

Q_SIGNALS:
     void sigSelectedStop(int stop);

protected:
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void mouseReleaseEvent(QMouseEvent * e);
    virtual void mouseMoveEvent(QMouseEvent * e);

private Q_SLOTS:

private:
    void insertStop(double t);
   
private:
    KoStopGradient* m_gradient;
    int m_selectedStop;
    bool m_drag;
};

#endif
