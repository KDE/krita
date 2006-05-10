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

#include "ko_gray_widget.h"

#include <QLayout>
#include <q3hbox.h>
#include <QLabel>
#include <QSpinBox>
#include <QColor>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kdebug.h>

#include "koFrameButton.h"
#include "koColorSlider.h"
#include <kcolordialog.h>

KoGrayWidget::KoGrayWidget(QWidget *parent, const char *name) : super(parent, name)
{
    m_ColorButton = new KDualColorButton(this);
    Q_CHECK_PTR(m_ColorButton);

    m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
    Q3GridLayout *mGrid = new Q3GridLayout(this, 3, 5, 5, 2);

    /* setup color sliders */
    mSlider = new KoColorSlider(this);
    mSlider->setFocusPolicy( Qt::ClickFocus );
    mSlider->setMaximumHeight(20);
    mSlider->slotSetRange(0, 255);
    mSlider->slotSetColor1(QColor(255, 255, 255));
    mSlider->slotSetColor2(QColor(0, 0, 0));

    /* setup slider labels */
    mLabel = new QLabel("K:", this);
    mLabel->setFixedWidth(12);
    mLabel->setFixedHeight(20);

    /* setup spin box */
    mIn = new QSpinBox(0, 255, 1, this);
    mIn->setFixedWidth(50);
    mIn->setFixedHeight(20);
    mIn->setFocusPolicy( Qt::ClickFocus );

    mGrid->addMultiCellWidget(m_ColorButton, 0, 3, 0, 0, Qt::AlignTop);
    mGrid->addWidget(mLabel, 0, 1);
    mGrid->addMultiCellWidget(mSlider, 0, 0, 2, 3);
    mGrid->addWidget(mIn, 0, 4);

    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    /* connect color slider */
    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));

    /* connect spin box */
    connect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
}

void KoGrayWidget::slotChanged(int v)
{
    v = 255 - v;

    if (m_ColorButton->current() == KDualColorButton::Foreground){
        slotFGColorSelected( QColor( v, v, v));
    }
    else{
        slotBGColorSelected( QColor( v, v, v));
    }
}

void KoGrayWidget::setFgColor(const QColor & c)
{
    blockSignals(true);
    slotFGColorSelected(c);
    blockSignals(false);
}

void KoGrayWidget::setBgColor(const QColor & c)
{
    blockSignals(true);
    slotBGColorSelected(c);
    blockSignals(false);
}

void KoGrayWidget::update(const QColor & fgColor, const QColor & bgColor)
{
    
    m_fgColor = fgColor;
    m_bgColor = bgColor;

    QColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

    disconnect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
    disconnect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));

    mIn->blockSignals(true);
    mSlider->blockSignals(true);
    double v = color.red() + color.green() + color.blue();
    v /= 3.0;
    v = 255.0 - v;
    mIn->setValue(static_cast<int>(v));
    mSlider->slotSetValue(static_cast<int>(v));
    mIn->blockSignals(false);
    mSlider->blockSignals(false);

    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
    connect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
}

void KoGrayWidget::slotFGColorSelected(const QColor& c)
{
    m_fgColor = c;

    disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    m_ColorButton->setForeground( m_fgColor );
    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));

    emit sigFgColorChanged(m_fgColor);
}

void KoGrayWidget::slotBGColorSelected(const QColor& c)
{
    m_bgColor = c;

    disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
    m_ColorButton->setBackground( m_bgColor );
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    emit sigBgColorChanged(m_bgColor);
}

void KoGrayWidget::currentChanged(KDualColorButton::DualColor s)
{
   if(s == KDualColorButton::Foreground)
     slotFGColorSelected(m_ColorButton->currentColor());
   else
     slotBGColorSelected(m_ColorButton->currentColor());
}

#include "ko_gray_widget.moc"
