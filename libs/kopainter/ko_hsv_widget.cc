/*
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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

#include "ko_hsv_widget.h"
#include "ko_color_wheel.h"

#include <kselector.h>
#include <QLayout>
#include <q3hbox.h>
#include <QLabel>
#include <QSpinBox>
#include <QToolTip>
#include <QGridLayout>
#include "koFrameButton.h"
#include "koColorSlider.h"
#include <kcolordialog.h>
#include <kdualcolorbutton.h>
#include "koColor.h"
#include <kdebug.h>
#include <klocale.h>

KoHSVWidget::KoHSVWidget(QWidget *parent, const char *name) : super(parent)
{
    setObjectName(name);

    m_ColorButton = new KDualColorButton(this);
    m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());

    QGridLayout *mGrid = new QGridLayout;
    m_colorwheel = new KoColorWheel(this);
    m_colorwheel->setFixedSize( 120, 120);
    m_VSelector = new KValueSelector(Qt::Vertical, this);
    m_VSelector-> setFixedSize( 30, 120);

    /* setup slider labels */
    mHLabel = new QLabel("H:", this);
    mHLabel->setFixedSize(12, 20);
    mSLabel = new QLabel("S:", this);
    mSLabel->setFixedSize(12, 20);
    mVLabel = new QLabel("V:", this);
    mVLabel->setFixedSize(12, 20);

    /* setup spin box */
    mHIn = new QSpinBox(this);
    mHIn->setMinimum(0);
    mHIn->setMaximum(359);   // Is this correct? 359?
    mHIn->setSingleStep(1);
    mHIn->setFixedSize(50, 20);
    mHIn->setFocusPolicy( Qt::ClickFocus );
    mHIn->setToolTip( i18n( "Hue" ) );

    mSIn = new QSpinBox(this);
    mSIn->setMinimum(0);
    mSIn->setMaximum(255);
    mSIn->setSingleStep(1);
    mSIn->setFixedSize(50, 20);
    mSIn->setFocusPolicy( Qt::ClickFocus );
    mSIn->setToolTip( i18n( "Saturation" ) );

    mVIn = new QSpinBox(this);
    mVIn->setMinimum(0);
    mVIn->setMaximum(255);
    mVIn->setSingleStep(1);
    mVIn->setFixedSize(50, 20);
    mVIn->setFocusPolicy( Qt::ClickFocus );
    mVIn->setToolTip( i18n( "Value (brightness)" ) );

    mGrid->addWidget(m_ColorButton, 0, 0, 2, 2, Qt::AlignTop);

    mGrid->addWidget(mHLabel, 2, 0, Qt::AlignTop);
    mGrid->addWidget(mSLabel, 3, 0, Qt::AlignTop);
    mGrid->addWidget(mVLabel, 4, 0, Qt::AlignTop);

    mGrid->addWidget(m_colorwheel, 0, 2, 5, 4, Qt::AlignTop);

    mGrid->addWidget(mHIn, 2, 1, Qt::AlignTop);
    mGrid->addWidget(mSIn, 3, 1, Qt::AlignTop);
    mGrid->addWidget(mVIn, 4, 1, Qt::AlignTop);

    mGrid->addWidget(m_VSelector, 0, 6, 5, 1, Qt::AlignTop);

    mGrid->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum ), 0, 7 );
    mGrid->addItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ), 5, 0 );


    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(currentChanged(KDualColorButton::DualColor)), this, SLOT(currentChanged(KDualColorButton::DualColor)));

    connect(m_VSelector, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
    connect(m_colorwheel, SIGNAL(valueChanged(const KoColor&)), this, SLOT(slotWheelChanged(const KoColor&)));

    /* connect spin box */
    connect(mHIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
    connect(mSIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
    connect(mVIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));

    //setFixedSize(mGrid -> minimumSize());
    m_autovalue = true; // So on the initial selection of h or v, s gets set to 255.

    update(QColor(Qt::black), QColor(Qt::white));

    setLayout(mGrid);
}

void KoHSVWidget::slotHChanged(int h)
{
    //kDebug() << "H changed: " << h << endl;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(h, m_fgColor.S(), m_fgColor.V());
        changedFgColor();
    }
    else{
        m_bgColor.setHSV(h, m_bgColor.S(), m_bgColor.V());
        changedBgColor();
    }
}

void KoHSVWidget::slotSChanged(int s)
{
    //kDebug() << "S changed: " << s << endl;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(m_fgColor.H(), s, m_fgColor.V());
        changedFgColor();
    }
    else{
        m_bgColor.setHSV(m_bgColor.H(), s, m_bgColor.V());
        changedBgColor();
    }
}

void KoHSVWidget::slotVChanged(int v)
{
    //kDebug() << "V changed: " << v << ", setting autovalue to false " << endl;
    m_autovalue = false;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(m_fgColor.H(), m_fgColor.S(), v);
        changedFgColor();
    }
    else{
        m_bgColor.setHSV(m_bgColor.H(), m_bgColor.S(), v);
        changedBgColor();
    }
}

void KoHSVWidget::slotWheelChanged(const KoColor& c)
{
    //kDebug() << "Wheel changed: " << c.color() <<  endl;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        if(m_autovalue)
            m_fgColor.setHSV(c.H(), c.S(), 255);
        else
            m_fgColor.setHSV(c.H(), c.S(), m_fgColor.V());
        changedFgColor();
    }
    else{
        if(m_autovalue)
            m_bgColor.setHSV(c.H(), c.S(), 255);
        else
            m_bgColor.setHSV(c.H(), c.S(), m_bgColor.V());
        changedBgColor();
    }
}


void KoHSVWidget::setFgColor(const QColor & c)
{
    //kDebug() << "setFGColor " << c << endl;
    blockSignals(true);
    slotFGColorSelected(c);
    blockSignals(false);
}

void KoHSVWidget::setBgColor(const QColor & c)
{
    //kDebug() << "setBgColor " << c << endl;
    blockSignals(true);
    slotBGColorSelected(c);
    blockSignals(false);
}

void KoHSVWidget::changedFgColor()
{
    //kDebug() << "ChangedFgColor\n";
    disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    m_ColorButton->setForeground( m_fgColor.color() );
    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));

    update( m_fgColor, m_bgColor);

    emit sigFgColorChanged(m_fgColor.color());
}

void KoHSVWidget::changedBgColor()
{
    //kDebug() << "changedBgColor()\n";
    disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
    m_ColorButton->setBackground( m_bgColor.color() );
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    update(m_fgColor, m_bgColor );

    emit sigBgColorChanged(m_bgColor.color());
}

void KoHSVWidget::update(const KoColor & fgColor, const KoColor & bgColor)
{
    
    mHIn->blockSignals(true);
    mSIn->blockSignals(true);
    mVIn->blockSignals(true);
    m_VSelector->blockSignals(true);
    m_colorwheel->blockSignals(true);
            
    //kDebug() << "update. FG: " << fgColor.color() << ", bg: " << bgColor.color() << endl;
    m_fgColor = fgColor;
    m_bgColor = bgColor;

    KoColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

    int h = color.H();
    int s = color.S();
    int v = color.V();

    mHIn->setValue(h);
    mSIn->setValue(s);
    mVIn->setValue(v);
    
    m_VSelector->setHue(h);
    m_VSelector->setSaturation(s);
    m_VSelector->setValue(v);
    m_VSelector->updateContents();
    
    m_colorwheel->slotSetValue(color);

    mHIn->blockSignals(false);
    mSIn->blockSignals(false);
    mVIn->blockSignals(false);
    m_VSelector->blockSignals(false);
    //m_VSelector->repaint(false);
    m_colorwheel->blockSignals(false);
}

void KoHSVWidget::slotFGColorSelected(const QColor& c)
{
    //kDebug() << "slotFGColorSelected " << c << endl;
    m_fgColor = KoColor(c);

    changedFgColor();
}

void KoHSVWidget::slotBGColorSelected(const QColor& c)
{
    //kDebug() << "slotBGColorSelected()" << c << endl;
    m_bgColor = KoColor(c);

    changedBgColor();
}

void KoHSVWidget::currentChanged(KDualColorButton::DualColor s)
{
    //kDebug() << "currentChanged\n";
    if(s == KDualColorButton::Foreground)
        slotFGColorSelected(m_ColorButton->currentColor());
    else
        slotBGColorSelected(m_ColorButton->currentColor());
}

#include "ko_hsv_widget.moc"
