/*
 *  kis_colorchooser.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter (elter@kde.org)
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

#ifndef __kis_colorchooser_h__
#define __kis_colorchooser_h__

#include "kis_color.h"

#include "kpixmap.h"
#include "kpixmapeffect.h"
#include "kis_util.h"

class QFrame;
class QLabel;
class QSpinBox;
class QSpinBox;
class RGBWidget;
class HSBWidget;
class CMYKWidget;
class LABWidget;
class GreyWidget;
class ColorFrame;
class ColorSlider;
class QVBoxLayout;


class KisColorChooser : public QWidget
{
  Q_OBJECT

 public:
 
    KisColorChooser(QWidget *parent = 0L);

 public slots:
 
    void slotShowGrey();
    void slotShowRGB();
    void slotShowHSB();
    void slotShowCMYK();
    void slotShowLAB();

    void slotSetFGColor(const KoColor&);
    void slotSetBGColor(const KoColor&);
    void slotSetActiveColor( ActiveColor a) { m_active = a; }

 protected slots:

    void slotRGBWidgetChanged(const KoColor&);
    void slotGreyWidgetChanged(const KoColor&);
    void slotColorFrameChanged(const QColor&);

 protected:

    virtual void resizeEvent(QResizeEvent *);

 signals:

    void colorChanged(const KoColor&);
    void hueChanged(int h);
    void satChanged(int s);
    void valChanged(int v);
    
 protected:
  
    ColorFrame       *m_pColorFrame;
    RGBWidget        *m_pRGBWidget;
    HSBWidget        *m_pHSBWidget;
    CMYKWidget       *m_pCMYKWidget;
    LABWidget        *m_pLABWidget;
    GreyWidget       *m_pGreyWidget;
    KoColor          m_fg, m_bg;
    ActiveColor       m_active;
};

class RGBWidget : public QWidget
{
  Q_OBJECT
 
 public:
    RGBWidget(QWidget *parent = 0L);

 public slots:
    void slotSetColor(const KoColor&);

 protected slots:
    void slotRSliderChanged(int);
    void slotGSliderChanged(int);
    void slotBSliderChanged(int);

    void slotHSliderChanged(int);
    void slotSSliderChanged(int);
    void slotVSliderChanged(int);

    void slotRInChanged(int);
    void slotGInChanged(int);
    void slotBInChanged(int);

    void slotHInChanged(int);
    void slotSInChanged(int);
    void slotVInChanged(int);

 signals:
    void colorChanged(const KoColor&);
    void hueChanged(int h);
    void satChanged(int s);
    void valChanged(int v);

 protected:
    virtual void resizeEvent(QResizeEvent *);

 protected:
    ColorSlider *m_pRSlider, *m_pGSlider,   *m_pBSlider;
    ColorSlider *m_pHSlider, *m_pSSlider,   *m_pVSlider;

    QLabel      *m_pRLabel,  *m_pGLabel,    *m_pBLabel;
    QLabel      *m_pHLabel,  *m_pSLabel,    *m_pVLabel;

    QSpinBox    *m_pRIn,     *m_pGIn,       *m_pBIn;
    QSpinBox    *m_pHIn,     *m_pSIn,       *m_pVIn;

    KoColor    m_c;
};

class GreyWidget : public QWidget
{
    Q_OBJECT
 
 public:
    GreyWidget(QWidget *parent = 0L);

 public slots:
    void slotSetColor(const KoColor&);

 protected slots:
    void slotVSliderChanged(int);
    void slotVInChanged(int);

 signals:
    void colorChanged(const KoColor&);

 protected:
    virtual void resizeEvent(QResizeEvent *);

 protected:
    ColorSlider *m_pVSlider;
    QLabel      *m_pVLabel;
    QSpinBox    *m_pVIn;
    KoColor     m_c;
};

#endif
