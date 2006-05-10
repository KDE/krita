/*
 * This file is part of KOffice
 *
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

#include "ko_cmyk_widget.h"

#include <QLayout>
#include <q3hbox.h>
#include <QLabel>
#include <QSpinBox>
#include <QToolTip>
#include <QColor>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "koFrameButton.h"
#include "koColorSlider.h"
#include <kcolordialog.h>
#include <kdualcolorbutton.h>

KoCMYKWidget::KoCMYKWidget(QWidget *parent, const char *name) : super(parent, name)
{
    m_ColorButton = new KDualColorButton(this);
    m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
    Q3GridLayout *mGrid = new Q3GridLayout(this, 4, 5, 5, 2);

    /* setup color sliders */
    mCSlider = new KoColorSlider(this);
    mCSlider->setMaximumHeight(20);
    mCSlider->slotSetRange(0, 255);
    mCSlider->setFocusPolicy( Qt::ClickFocus );

    mMSlider = new KoColorSlider(this);
    mMSlider->setMaximumHeight(20);
    mMSlider->slotSetRange(0, 255);
    mMSlider->setFocusPolicy( Qt::ClickFocus );

    mYSlider = new KoColorSlider(this);
    mYSlider->setMaximumHeight(20);
    mYSlider->slotSetRange(0, 255);
    mYSlider->setFocusPolicy( Qt::ClickFocus );

    mKSlider = new KoColorSlider(this);
    mKSlider->setMaximumHeight(20);
    mKSlider->slotSetRange(0, 255);
    mKSlider->setFocusPolicy( Qt::ClickFocus );

    /* setup slider labels */
    mCLabel = new QLabel("C", this);
    mCLabel->setFixedWidth(12);
    mCLabel->setFixedHeight(20);
    mMLabel = new QLabel("M", this);
    mMLabel->setFixedWidth(12);
    mMLabel->setFixedHeight(20);
    mYLabel = new QLabel("Y", this);
    mYLabel->setFixedWidth(12);
    mYLabel->setFixedHeight(20);
    mKLabel = new QLabel("K", this);
    mKLabel->setFixedWidth(12);
    mKLabel->setFixedHeight(20);

    /* setup spin box */
    mCIn = new QSpinBox(0, 255, 1, this);
    mCIn->setFixedWidth(50);
    mCIn->setFixedHeight(20);
    mCIn->setFocusPolicy( Qt::ClickFocus );
    mCIn->setToolTip( i18n( "Cyan" ) );

    mMIn = new QSpinBox(0, 255, 1, this);
    mMIn->setFixedWidth(50);
    mMIn->setFixedHeight(20);
    mMIn->setFocusPolicy( Qt::ClickFocus );
    mMIn->setToolTip( i18n( "Magenta" ) );

    mYIn = new QSpinBox(0, 255, 1, this);
    mYIn->setFixedWidth(50);
    mYIn->setFixedHeight(20);
    mYIn->setFocusPolicy( Qt::ClickFocus );
    mYIn->setToolTip( i18n( "Yellow" ) );

    mKIn = new QSpinBox(0, 255, 1, this);
    mKIn->setFixedWidth(50);
    mKIn->setFixedHeight(20);
    mKIn->setFocusPolicy( Qt::ClickFocus );
    mKIn->setToolTip( i18n( "Black" ) );

    mGrid->addMultiCellWidget(m_ColorButton, 0, 4, 0, 0, Qt::AlignTop);
    mGrid->addWidget(mCLabel, 0, 1);
    mGrid->addWidget(mMLabel, 1, 1);
    mGrid->addWidget(mYLabel, 2, 1);
    mGrid->addWidget(mKLabel, 3, 1);
    mGrid->addMultiCellWidget(mCSlider, 0, 0, 2, 3);
    mGrid->addMultiCellWidget(mMSlider, 1, 1, 2, 3);
    mGrid->addMultiCellWidget(mYSlider, 2, 2, 2, 3);
    mGrid->addMultiCellWidget(mKSlider, 3, 3, 2, 3);
    mGrid->addWidget(mCIn, 0, 4);
    mGrid->addWidget(mMIn, 1, 4);
    mGrid->addWidget(mYIn, 2, 4);
    mGrid->addWidget(mKIn, 3, 4);

    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    /* connect color sliders */
    connect(mCSlider, SIGNAL(valueChanged(int)), this, SLOT(slotCChanged(int)));
    connect(mMSlider, SIGNAL(valueChanged(int)), this, SLOT(slotMChanged(int)));
    connect(mYSlider, SIGNAL(valueChanged(int)), this, SLOT(slotYChanged(int)));
    connect(mKSlider, SIGNAL(valueChanged(int)), this, SLOT(slotKChanged(int)));

    /* connect spin box */
    connect(mCIn, SIGNAL(valueChanged(int)), this, SLOT(slotCChanged(int)));
    connect(mMIn, SIGNAL(valueChanged(int)), this, SLOT(slotMChanged(int)));
    connect(mYIn, SIGNAL(valueChanged(int)), this, SLOT(slotYChanged(int)));
	connect(mKIn, SIGNAL(valueChanged(int)), this, SLOT(slotKChanged(int)));
}

void KoCMYKWidget::slotCChanged(int c)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        CMYKColor col = RgbToCmyk(m_fgColor);
        col.C = c / 255.0;
        m_fgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        emit sigFgColorChanged(m_fgColor);
    }
    else{
        CMYKColor col = RgbToCmyk(m_bgColor);
        col.C = c / 255.0;
        m_bgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Background);
        emit sigBgColorChanged(m_bgColor);
    }
}

void KoCMYKWidget::slotMChanged(int m)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        CMYKColor col = RgbToCmyk(m_fgColor);
        col.M = m / 255.0;
        m_fgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        emit sigFgColorChanged(m_fgColor);
    }
    else{
        CMYKColor col = RgbToCmyk(m_bgColor);
        col.M = m / 255.0;
        m_bgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Background);
        emit sigBgColorChanged(m_bgColor);
    }
}

void KoCMYKWidget::slotYChanged(int y)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        CMYKColor col = RgbToCmyk(m_fgColor);
        col.Y = y / 255.0;
        m_fgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        emit sigFgColorChanged(m_fgColor);
    }
    else{
        CMYKColor col = RgbToCmyk(m_bgColor);
        col.Y = y / 255.0;
        m_bgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Background);
        emit sigBgColorChanged(m_bgColor);
    }
}

void KoCMYKWidget::slotKChanged(int k)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        CMYKColor col = RgbToCmyk(m_fgColor);
        col.K = k / 255.0;
        m_fgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        emit sigFgColorChanged(m_fgColor);
    }
    else{
        CMYKColor col = RgbToCmyk(m_bgColor);
        col.K = k / 255.0;
        m_bgColor = CmykToRgb(col);
        m_ColorButton->setCurrent(KDualColorButton::Background);
        emit sigBgColorChanged(m_bgColor);
    }
}

void KoCMYKWidget::setFgColor(const QColor & c)
{
    update(c, m_bgColor);
}

void KoCMYKWidget::setBgColor(const QColor & c)
{
    update(m_fgColor, c);
}

void KoCMYKWidget::update(const QColor fgColor, const QColor bgColor)
{
    m_fgColor = fgColor;
    m_bgColor = bgColor;

    QColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

    CMYKColor col = RgbToCmyk(color);

    disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    m_ColorButton->setForeground( m_fgColor );
    m_ColorButton->setBackground( m_bgColor );

    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    mCSlider->blockSignals(true);
    mCIn->blockSignals(true);
    mMSlider->blockSignals(true);
    mMIn->blockSignals(true);
    mYSlider->blockSignals(true);
    mYIn->blockSignals(true);
    mKSlider->blockSignals(true);
    mKIn->blockSignals(true);

    CMYKColor minC = col;
    minC.C = 0.0;
    CMYKColor maxC = col;
    maxC.C = 1.0;

    mCSlider->slotSetColor1(CmykToRgb(minC));
    mCSlider->slotSetColor2(CmykToRgb(maxC));
    mCSlider->slotSetValue(int(col.C * 255));
    mCIn->setValue(int(col.C * 255));

    CMYKColor minM = col;
    minM.M = 0.0;
    CMYKColor maxM = col;
    maxM.M = 1.0;

    mMSlider->slotSetColor1(CmykToRgb(minM));
    mMSlider->slotSetColor2(CmykToRgb(maxM));
    mMSlider->slotSetValue(int(col.M * 255));
    mMIn->setValue(int(col.M * 255));

    CMYKColor minY = col;
    minY.Y = 0.0;
    CMYKColor maxY = col;
    maxY.Y = 1.0;

    mYSlider->slotSetColor1(CmykToRgb(minY));
    mYSlider->slotSetColor2(CmykToRgb(maxY));
    mYSlider->slotSetValue(int(col.Y * 255));
    mYIn->setValue(int(col.Y * 255));

    CMYKColor minK = col;
    minK.K = 0.0;
    CMYKColor maxK = col;
    maxK.K = 1.0;

    mKSlider->slotSetColor1(CmykToRgb(minK));
    mKSlider->slotSetColor2(CmykToRgb(maxK));
    mKSlider->slotSetValue(int(col.K * 255));
    mKIn->setValue(int(col.K * 255));

    mCSlider->blockSignals(false);
    mCIn->blockSignals(false);
    mMSlider->blockSignals(false);
    mMIn->blockSignals(false);
    mYSlider->blockSignals(false);
    mYIn->blockSignals(false);
    mKSlider->blockSignals(false);
    mKIn->blockSignals(false);
}

void KoCMYKWidget::slotFGColorSelected(const QColor& c)
{
    m_fgColor = QColor(c);
    emit sigFgColorChanged(m_fgColor);
}

void KoCMYKWidget::slotBGColorSelected(const QColor& c)
{
    m_bgColor = QColor(c);
    emit sigBgColorChanged(m_bgColor);
}

CMYKColor KoCMYKWidget::RgbToCmyk(const QColor& col)
{
	kDebug() << "--[ KoCMYKWidget::RgbToCmyk ]--------------------------------------" << endl;
	kDebug() << endl;

    // RGB to CMY
    float r = col.red() / 255.0;
    float g = col.green() / 255.0;
    float b = col.blue() / 255.0;

	kDebug() << "float r = col.red() / 255.0;" << endl;
	kDebug() << "      r = " << col.red() << " / 255.0 = " << r << endl;
	kDebug() << "float g = col.green() / 255.0;" << endl;
	kDebug() << "      g = " << col.green() << " / 255.0 = " << g << endl;
	kDebug() << "float b = col.blue() / 255.0;" << endl;
	kDebug() << "      b = " << col.blue() << " / 255.0 = " << b << endl;
	kDebug() << endl;

    float ac = 1.0 - r;
    float am = 1.0 - g;
    float ay = 1.0 - b;

	kDebug() << "float ac = 1.0 - r;" << endl;
	kDebug() << "      ac = 1.0 - " << r << " = " << ac << endl;
	kDebug() << "float am = 1.0 - g;" << endl;
	kDebug() << "      am = 1.0 - " << g << " = " << am << endl;
	kDebug() << "float ay = 1.0 - b;" << endl;
	kDebug() << "      ay = 1.0 - " << b << " = " << ay << endl;
	kDebug() << endl;

    // CMY to CMYK
    float c = 0.0;
    float m = 0.0;
    float y = 0.0;
    float k = 0.0;

    if ((r == 0.0) && (g == 0.0) && (b == 0.0))
    {
		kDebug() << "r = g = b = 0.0: Therefor k = 1.0" << endl;
        k = 1.0;
    }
    else
    {
		kDebug() << "r = g = b != 0.0: Therefor k = min(ac,am,ay)" << endl;

        if (qMin(ac,am) == ac)
            k = qMin(ac,ay);
        else
            k = qMin(am,ay);

        c = (ac - k) / (1.0 - k);
        m = (am - k) / (1.0 - k);
        y = (ay - k) / (1.0 - k);
    }

	kDebug() << "float k = " << k << endl;
	kDebug() << endl;

	kDebug() << "float c = (ac - k) / (1.0 - k);" << endl;
	kDebug() << "      c = (" << ac << " - " << k << ") / (1.0 - " << k << ") = " << c << endl;
	kDebug() << "float m = (am - k) / (1.0 - k);" << endl;
	kDebug() << "      m = (" << am << " - " << k << ") / (1.0 - " << k << ") = " << m << endl;
	kDebug() << "float y = (ay - k) / (1.0 - k);" << endl;
	kDebug() << "      y = (" << ay << " - " << k << ") / (1.0 - " << k << ") = " << y << endl;
	kDebug() << endl;

    CMYKColor color;
    color.C = c;
    color.M = m;
    color.Y = y;
    color.K = k;

	kDebug() << "===================================================================" << endl;

    return color;
}

QColor KoCMYKWidget::CmykToRgb(const CMYKColor& c)
{
    // CMYK to CMY
    float ac = qMin(1.0, c.C * (1.0 - c.K) + c.K);
    float am = qMin(1.0, c.M * (1.0 - c.K) + c.K);
    float ay = qMin(1.0, c.Y * (1.0 - c.K) + c.K);

    // CMY to RGB
    int r = int((1.0 - ac) * 255.0);
    int g = int((1.0 - am) * 255.0);
    int b = int((1.0 - ay) * 255.0);

    QColor color;
    color.setRgb(r,g,b);

    return color;
}

#include "ko_cmyk_widget.moc"
