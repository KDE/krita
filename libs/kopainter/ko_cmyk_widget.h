/* This file is part of the KDE project
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
 * Copyright (c) 2005 Tim Beaulen (tbscope@gmail.com)
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
#ifndef KO_CMYK_WIDGET_H
#define KO_CMYK_WIDGET_H

#include "qwidget.h"
//Added by qt3to4:
#include <QLabel>
#include <Q3GridLayout>

#include <koffice_export.h>

class KoFrameButton;
class Q3GridLayout;
class QColor;
class KoColorSlider;
class QLabel;
class QSpinBox;
class KDualColorButton;

struct CMYKColor
{
    float C;
    float M;
    float Y;
    float K;
};

class KoCMYKWidget
     : public QWidget
{
    Q_OBJECT
    typedef QWidget super;

public:
    KoCMYKWidget(QWidget *parent = 0L, const char *name = 0);
    virtual ~KoCMYKWidget() {}

public slots:
    /**
     * Set the current color to c. Do not emit the color changed signals
     */
    virtual void setFgColor(const QColor & c);
    virtual void setBgColor(const QColor & c);

signals:

    /**
     * Emitted when the current color is changed.
     */
    void sigFgColorChanged(const QColor & c);
    void sigBgColorChanged(const QColor & c);


protected slots:

    virtual void slotCChanged(int c);
    virtual void slotMChanged(int m);
    virtual void slotYChanged(int y);
    virtual void slotKChanged(int k);

    void slotFGColorSelected(const QColor& c);
    void slotBGColorSelected(const QColor& c);

private:

    void update(const QColor fgColor, const QColor);

    CMYKColor RgbToCmyk(const QColor& col);
    QColor CmykToRgb(const CMYKColor& c);

private:

    KoColorSlider *mCSlider;
    KoColorSlider *mMSlider;
    KoColorSlider *mYSlider;
    KoColorSlider *mKSlider;
    QLabel *mCLabel;
    QLabel *mMLabel;
    QLabel *mYLabel;
    QLabel *mKLabel;
    QSpinBox *mCIn;
    QSpinBox *mMIn;
    QSpinBox *mYIn;
    QSpinBox *mKIn;
    KDualColorButton *m_ColorButton;

    float m_fgC;
    float m_fgM;
    float m_fgY;
    float m_fgK;

    float m_bgC;
    float m_bgM;
    float m_bgY;
    float m_bgK;

    QColor m_fgColor;
    QColor m_bgColor;
};

#endif
