/*
 *  colorslider.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __colorslider_h__
#define __colorslider_h__

#include <qpoint.h>
#include <qwidget.h>

class ColorFrame;
class SliderWidget;

class ColorSlider : public QWidget
{
    Q_OBJECT
 
    public:
        ColorSlider(QWidget *parent = 0L, int _colorSliderType = 0);
        virtual ~ColorSlider();

        int minValue();
        int maxValue();

    protected:
        virtual void resizeEvent (QResizeEvent *);
        virtual void mousePressEvent (QMouseEvent *);
  
    public slots:
        void slotSetColor1(const QColor&);
        void slotSetColor2(const QColor&);
        void slotSetHue(int _hue);
        void slotSetSaturation(int _sat);
        void slotSetVal(int _val);

        void slotSetValue(int);
        void slotSetRange(int min, int max);
  
    protected slots:
        void slotSliderMoved(int);
        void slotFrameClicked(const QPoint&);

    signals:
        void colorSelected(const QColor&);
        void valueChanged(int);

    protected:
        SliderWidget    *m_pSlider;
        ColorFrame      *m_pColorFrame;
        int             m_ColorSliderType;
        int             m_min, m_max;
        int             m_value;
};

class SliderWidget : public QWidget
{
    Q_OBJECT
 
    public:
        SliderWidget(QWidget *parent = 0L);

    protected:
        virtual void mousePressEvent (QMouseEvent *); 
        virtual void mouseReleaseEvent (QMouseEvent *); 
        virtual void mouseMoveEvent (QMouseEvent *);
        virtual void paintEvent (QPaintEvent *);
  
    signals:
        void  positionChanged(int);

    protected:
        bool   m_dragging;
        QPoint m_myPos;
};

#endif
