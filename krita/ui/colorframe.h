/*
 *  colorframe.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter
 *            (c) 2001 John Califf
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

#ifndef __colorframe_h__
#define __colorframe_h__

#include <qrect.h>
#include <qframe.h>
#include <qimage.h>

#include "kpixmap.h"
#include "kpixmapeffect.h"

class ColorFrame : public QFrame
{
    Q_OBJECT
 
    public:
        ColorFrame(QWidget *parent = 0L, int _colorFrameType = 0);
        const QColor colorAt(const QPoint&);
  
    protected:
        virtual void drawContents (QPainter *);
        virtual void mousePressEvent (QMouseEvent *);
        virtual void mouseMoveEvent (QMouseEvent *);
        virtual void mouseReleaseEvent (QMouseEvent *);
        
        void drawHueFrame(QPixmap *pixmap);  
        void drawSaturationFrame(QPixmap *pixmap);  
        void drawValueFrame(QPixmap *pixmap);  

    public slots:
        void slotSetColor1(const QColor&);
        void slotSetColor2(const QColor&);
        void slotSetHue(int _hue);
        void slotSetSaturation(int _sat);
        void slotSetValue(int _val);

    signals:
        void clicked(const QPoint&);
        void colorSelected(const QColor&);

    protected:
        QColor      m_c1, m_c2;
        int         m_Hue, m_Saturation, m_Value;
        
        KPixmap     m_pm;
        QImage      m_pmImage;
        
        bool        m_colorChanged;
        bool        m_pixChanged;
        bool        m_dragging;
        int         m_ColorFrameType;
};

#endif
