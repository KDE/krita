/*
 * This file is part of Krita
 *
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

#include "kis_hsv_widget.h"
#include "kis_colorwheel.h"

#include <kselect.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>
#include <kdualcolorbutton.h>
#include <koColor.h>
#include <kdebug.h>
#include "kis_color.h"
#include "kis_canvas_subject.h"
#include "kis_factory.h"
#include "kis_colorspace_factory_registry.h"
#include <kis_meta_registry.h>

KisHSVWidget::KisHSVWidget(QWidget *parent, const char *name) : super(parent, name)
{
    m_subject = 0;

    m_ColorButton = new KDualColorButton(this);
    m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());

    QGridLayout *mGrid = new QGridLayout(this, 5, 7, 5, 2);
    m_colorwheel = new KisColorWheel(this);
    m_colorwheel->setFixedSize( 120, 120);
    m_VSelector = new KValueSelector(Qt::Vertical, this);
    m_VSelector-> setFixedSize( 30, 120);


    /* setup slider labels */
    mHLabel = new QLabel("H", this);
    mHLabel->setFixedSize(12, 20);
    mSLabel = new QLabel("S", this);
    mSLabel->setFixedSize(12, 20);
    mVLabel = new QLabel("V", this);
    mVLabel->setFixedSize(12, 20);

    /* setup spin box */
    mHIn = new QSpinBox(0, 359, 1, this);
    mHIn->setFixedSize(50, 20);
    mHIn->setFocusPolicy( QWidget::ClickFocus );

    mSIn = new QSpinBox(0, 255, 1, this);
    mSIn->setFixedSize(50, 20);
    mSIn->setFocusPolicy( QWidget::ClickFocus );

    mVIn = new QSpinBox(0, 255, 1, this);
    mVIn->setFixedSize(50, 20);
    mVIn->setFocusPolicy( QWidget::ClickFocus );

    mGrid->addMultiCellWidget(m_ColorButton, 0, 0, 0, 1, Qt::AlignTop);

    mGrid->addWidget(mHLabel, 1, 0);
    mGrid->addWidget(mSLabel, 2, 0);
    mGrid->addWidget(mVLabel, 3, 0);

    mGrid->addMultiCellWidget(m_colorwheel, 0, 3, 2, 4);

    mGrid->addWidget(mHIn, 1, 1);
    mGrid->addWidget(mSIn, 2, 1);
    mGrid->addWidget(mVIn, 3, 1);

    mGrid->addMultiCellWidget(m_VSelector, 0, 3, 5, 5);


    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    connect(m_VSelector, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
    connect(m_colorwheel, SIGNAL(valueChanged(const KoColor&)), this, SLOT(slotWheelChanged(const KoColor&)));

    /* connect spin box */
    connect(mHIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
    connect(mSIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
    connect(mVIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));

    //setFixedSize(mGrid -> minimumSize());
    locked = false;
    autovalue = true;
}

void KisHSVWidget::slotHChanged(int h)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();

    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(h, m_fgColor.S(), m_fgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        if(m_subject)
            m_subject->setFGColor(KisColor(m_fgColor.color(), cs));
    }
    else{
        m_bgColor.setHSV(h, m_bgColor.S(), m_bgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Background);
        if(m_subject)
            m_subject->setBGColor(KisColor(m_bgColor.color(), cs));
    }
}

void KisHSVWidget::slotSChanged(int s)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(m_fgColor.H(), s, m_fgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        if(m_subject)
            m_subject->setFGColor(KisColor(m_fgColor.color(), cs));
    }
    else{
        m_bgColor.setHSV(m_bgColor.H(), s, m_bgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Background);
        if(m_subject)
            m_subject->setBGColor(KisColor(m_bgColor.color(), cs));
    }
}

void KisHSVWidget::slotVChanged(int v)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    autovalue = false;
    locked = true;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        m_fgColor.setHSV(m_fgColor.H(), m_fgColor.S(), v);
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        if(m_subject)
            m_subject->setFGColor(KisColor(m_fgColor.color(), cs));
    }
    else{
        m_bgColor.setHSV(m_bgColor.H(), m_bgColor.S(), v);
        m_ColorButton->setCurrent(KDualColorButton::Background);
        if(m_subject)
            m_subject->setBGColor(KisColor(m_bgColor.color(), cs));
    }
    locked = false;
}

void KisHSVWidget::slotWheelChanged(const KoColor& c)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    locked = true;
    if (m_ColorButton->current() == KDualColorButton::Foreground){
        if(autovalue)
            m_fgColor.setHSV(c.H(), c.S(), 255);
        else
            m_fgColor.setHSV(c.H(), c.S(), m_fgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Foreground);
        // XXX: I once got a crash here. BSAR.
        if(m_subject)
            m_subject->setFGColor(KisColor(m_fgColor.color(), cs));
    }
    else{
        if(autovalue)
            m_bgColor.setHSV(c.H(), c.S(), 255);
        else
            m_bgColor.setHSV(c.H(), c.S(), m_fgColor.V());
        m_ColorButton->setCurrent(KDualColorButton::Background);
        if(m_subject)
            m_subject->setBGColor(KisColor(m_bgColor.color(), cs));
    }
    locked = false;
}

void KisHSVWidget::update(KisCanvasSubject *subject)
{
    if( !locked )
    {
        m_subject = subject;
        m_fgColor = subject->fgColor().toQColor();
        m_bgColor = subject->bgColor().toQColor();
    }

    KoColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

    int h = color.H();
    int s = color.S();
    int v = color.V();

    m_ColorButton->blockSignals(true);
    m_ColorButton->setForeground( m_fgColor.color() );
    m_ColorButton->setBackground( m_bgColor.color() );
    m_ColorButton->blockSignals(false);

    mHIn->blockSignals(true);
    mSIn->blockSignals(true);
    mVIn->blockSignals(true);
    mHIn->setValue(h);
    mSIn->setValue(s);
    mVIn->setValue(v);
    mHIn->blockSignals(false);
    mSIn->blockSignals(false);
    mVIn->blockSignals(false);

    m_VSelector->blockSignals(true);
    m_VSelector->setHue(h);
    m_VSelector->setSaturation(s);
    m_VSelector->setValue(v);
    m_VSelector->updateContents();
    m_VSelector->blockSignals(false);
    m_VSelector->repaint(false);

    m_colorwheel->blockSignals(true);
    m_colorwheel->slotSetValue(color);
    m_colorwheel->blockSignals(false);
}

void KisHSVWidget::slotFGColorSelected(const QColor& c)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    m_fgColor = KoColor(c);
    if(m_subject)
    {
        QColor bgColor = m_ColorButton -> background();
        m_subject->setFGColor(KisColor(m_fgColor.color(), cs));
        //Background signal could be blocked so do that manually
        //see bug 106919
        m_subject->setBGColor(KisColor(bgColor, cs));
    }
}

void KisHSVWidget::slotBGColorSelected(const QColor& c)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    m_bgColor = KoColor(c);
    if(m_subject)
        m_subject->setBGColor(KisColor(m_bgColor.color(), cs));
}

#include "kis_hsv_widget.moc"
