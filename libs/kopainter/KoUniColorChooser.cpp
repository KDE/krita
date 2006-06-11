/*
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kselector.h>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QToolTip>
#include <QGridLayout>
#include <kcolordialog.h>

#include "koColor.h"
#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceFactoryRegistry.h"
#include "KoXYColorSelector.h"
#include "KoColorSlider.h"

#include "KoUniColorChooser.h"

KoUniColorChooser::KoUniColorChooser(KoColorSpaceFactoryRegistry* csFactoryRegistry, QWidget *parent, const char *name) : super(parent)
{
    setObjectName(name);

    QGridLayout *mGrid = new QGridLayout;

    m_xycolorselector = new KoXYColorSelector(csFactoryRegistry->getRGB8(), this);
    m_xycolorselector->setFixedSize(120, 120);

    m_colorSlider = new KoColorSlider(csFactoryRegistry->getRGB8(), Qt::Vertical, this);
    m_colorSlider->setFixedSize(25, 100);

    m_colorpatch = new QFrame(this);
    m_colorpatch->setFixedSize(18, 18);
    m_colorpatch->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    /* setup channel labels */
    m_HLabel = new QLabel("H:", this);
    m_HLabel->setFixedSize(10, 18);
    m_SLabel = new QLabel("S:", this);
    m_SLabel->setFixedSize(10, 18);
    m_VLabel = new QLabel("V:", this);
    m_VLabel->setFixedSize(10, 18);
    m_RLabel = new QLabel("R:", this);
    m_RLabel->setFixedSize(10, 18);
    m_GLabel = new QLabel("G:", this);
    m_GLabel->setFixedSize(10, 18);
    m_BLabel = new QLabel("B:", this);
    m_BLabel->setFixedSize(10, 18);
    m_LLabel = new QLabel("L:", this);
    m_LLabel->setFixedSize(10, 18);
    m_aLabel = new QLabel("a:", this);
    m_aLabel->setFixedSize(10, 18);
    m_bLabel = new QLabel("b:", this);
    m_bLabel->setFixedSize(10, 18);

    /* setup sradio buttons */
    m_HRB = new QRadioButton(this);
    m_SRB = new QRadioButton(this);
    m_VRB = new QRadioButton(this);
    m_RRB = new QRadioButton(this);
    m_GRB = new QRadioButton(this);
    m_BRB = new QRadioButton(this);
    m_LRB = new QRadioButton(this);
    m_aRB = new QRadioButton(this);
    m_bRB = new QRadioButton(this);
    
    /* setup spin box */
    m_HIn = new QSpinBox(this);
    m_HIn->setMinimum(0);
    m_HIn->setMaximum(359);   // Is this correct? 359?
    m_HIn->setSingleStep(1);
    m_HIn->setFixedSize(40, 18);
    m_HIn->setFocusPolicy( Qt::ClickFocus );
    m_HIn->setToolTip( i18n( "Hue" ) );

    m_SIn = new QSpinBox(this);
    m_SIn->setMinimum(0);
    m_SIn->setMaximum(255);
    m_SIn->setSingleStep(1);
    m_SIn->setFixedSize(40, 18);
    m_SIn->setFocusPolicy( Qt::ClickFocus );
    m_SIn->setToolTip( i18n( "Saturation" ) );

    m_VIn = new QSpinBox(this);
    m_VIn->setMinimum(0);
    m_VIn->setMaximum(255);
    m_VIn->setSingleStep(1);
    m_VIn->setFixedSize(40, 18);
    m_VIn->setFocusPolicy( Qt::ClickFocus );
    m_VIn->setToolTip( i18n( "Value (brightness)" ) );

    m_RIn = new QSpinBox(this);
    m_RIn->setMinimum(0);
    m_RIn->setMaximum(255);
    m_RIn->setSingleStep(1);
    m_RIn->setFixedSize(40, 18);
    m_RIn->setFocusPolicy( Qt::ClickFocus );
    m_RIn->setToolTip( i18n( "Red" ) );

    m_GIn = new QSpinBox(this);
    m_GIn->setMinimum(0);
    m_GIn->setMaximum(255);
    m_GIn->setSingleStep(1);
    m_GIn->setFixedSize(40, 18);
    m_GIn->setFocusPolicy( Qt::ClickFocus );
    m_GIn->setToolTip( i18n( "Green" ) );

    m_BIn = new QSpinBox(this);
    m_BIn->setMinimum(0);
    m_BIn->setMaximum(255);
    m_BIn->setSingleStep(1);
    m_BIn->setFixedSize(40, 18);
    m_BIn->setFocusPolicy( Qt::ClickFocus );
    m_BIn->setToolTip( i18n( "Blue" ) );

    m_LIn = new QSpinBox(this);
    m_LIn->setMinimum(0);
    m_LIn->setMaximum(100);
    m_LIn->setSingleStep(1);
    m_LIn->setFixedSize(40, 18);
    m_LIn->setFocusPolicy( Qt::ClickFocus );
    m_LIn->setToolTip( i18n( "Lightness" ) );

    m_aIn = new QSpinBox(this);
    m_aIn->setMinimum(0);
    m_aIn->setMaximum(255);
    m_aIn->setSingleStep(1);
    m_aIn->setFixedSize(40, 18);
    m_aIn->setFocusPolicy( Qt::ClickFocus );
    m_aIn->setToolTip( i18n( "Green to magenta*" ) );

    m_bIn = new QSpinBox(this);
    m_bIn->setMinimum(0);
    m_bIn->setMaximum(255);
    m_bIn->setSingleStep(1);
    m_bIn->setFixedSize(40, 18);
    m_bIn->setFocusPolicy( Qt::ClickFocus );
    m_bIn->setToolTip( i18n( "Blue to yellow" ) );

    mGrid->setSpacing(0);
    mGrid->setMargin(0);

    mGrid->addWidget(m_xycolorselector, 0, 0, -1, 1, Qt::AlignTop);

    mGrid->addWidget(m_colorpatch, 0, 1, Qt::AlignCenter);

    mGrid->addWidget(m_colorSlider, 1, 1, -1, 1, Qt::AlignTop);

    mGrid->addWidget(m_HRB, 0, 2, Qt::AlignCenter);
    mGrid->addWidget(m_SRB, 1, 2, Qt::AlignCenter);
    mGrid->addWidget(m_VRB, 2, 2, Qt::AlignCenter);
    mGrid->addWidget(m_RRB, 4, 2, Qt::AlignCenter);
    mGrid->addWidget(m_GRB, 5, 2, Qt::AlignCenter);
    mGrid->addWidget(m_BRB, 6, 2, Qt::AlignCenter);
    mGrid->addWidget(m_LRB, 0, 6, Qt::AlignCenter);
    mGrid->addWidget(m_aRB, 1, 6, Qt::AlignCenter);
    mGrid->addWidget(m_bRB, 2, 6, Qt::AlignCenter);

    mGrid->addWidget(m_HLabel, 0, 3, Qt::AlignTop);
    mGrid->addWidget(m_SLabel, 1, 3, Qt::AlignTop);
    mGrid->addWidget(m_VLabel, 2, 3, Qt::AlignTop);
    mGrid->addWidget(m_RLabel, 4, 3, Qt::AlignTop);
    mGrid->addWidget(m_GLabel, 5, 3, Qt::AlignTop);
    mGrid->addWidget(m_BLabel, 6, 3, Qt::AlignTop);
    mGrid->addWidget(m_LLabel, 0, 7, Qt::AlignTop);
    mGrid->addWidget(m_aLabel, 1, 7, Qt::AlignTop);
    mGrid->addWidget(m_bLabel, 2, 7, Qt::AlignTop);

    mGrid->addWidget(m_HIn, 0, 4, Qt::AlignTop);
    mGrid->addWidget(m_SIn, 1, 4, Qt::AlignTop);
    mGrid->addWidget(m_VIn, 2, 4, Qt::AlignTop);
    mGrid->addWidget(m_RIn, 4, 4, Qt::AlignTop);
    mGrid->addWidget(m_GIn, 5, 4, Qt::AlignTop);
    mGrid->addWidget(m_BIn, 6, 4, Qt::AlignTop);
    mGrid->addWidget(m_LIn, 0, 8, Qt::AlignTop);
    mGrid->addWidget(m_aIn, 1, 8, Qt::AlignTop);
    mGrid->addWidget(m_bIn, 2, 8, Qt::AlignTop);

    mGrid->addItem( new QSpacerItem( 4, 4, QSizePolicy::Fixed, QSizePolicy::Fixed), 3, 5 );
    mGrid->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding ), 7, 9 );

    //connect(m_VSelector, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
    //connect(m_colorwheel, SIGNAL(valueChanged(const KoOldColor&)), this, SLOT(slotWheelChanged(const KoOldColor&)));

    /* connect spin box */
    connect(m_HIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
    connect(m_SIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
    connect(m_VIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));

    //setFixedSize(mGrid -> minimumSize());
    m_autovalue = true; // So on the initial selection of h or v, s gets set to 255.

    update(QColor(Qt::black));

    setLayout(mGrid);
}

void KoUniColorChooser::slotHChanged(int h)
{
    //kDebug() << "H changed: " << h << endl;
    m_fgColor.setHSV(h, m_fgColor.S(), m_fgColor.V());
    changedFgColor();
}

void KoUniColorChooser::slotSChanged(int s)
{
    //kDebug() << "S changed: " << s << endl;
    m_fgColor.setHSV(m_fgColor.H(), s, m_fgColor.V());
    changedFgColor();
}

void KoUniColorChooser::slotVChanged(int v)
{
    //kDebug() << "V changed: " << v << ", setting autovalue to false " << endl;
    m_autovalue = false;
    m_fgColor.setHSV(m_fgColor.H(), m_fgColor.S(), v);
    changedFgColor();
}

void KoUniColorChooser::slotWheelChanged(const KoOldColor& c)
{
    //kDebug() << "Wheel changed: " << c.color() <<  endl;
    if(m_autovalue)
        m_fgColor.setHSV(c.H(), c.S(), 255);
    else
        m_fgColor.setHSV(c.H(), c.S(), m_fgColor.V());
    changedFgColor();
}


void KoUniColorChooser::setColor(const QColor & c)
{
    //kDebug() << "setFGColor " << c << endl;
    blockSignals(true);
    slotSetColor(c);
    blockSignals(false);
}

void KoUniColorChooser::changedFgColor()
{
    //kDebug() << "ChangedFgColor\n";
    update( m_fgColor);

    emit sigColorChanged(m_fgColor.color());
}


void KoUniColorChooser::update(const KoOldColor & fgColor)
{
    
    m_HIn->blockSignals(true);
    m_SIn->blockSignals(true);
    m_VIn->blockSignals(true);
    m_colorSlider->blockSignals(true);
    m_xycolorselector->blockSignals(true);
            
    //kDebug() << "update. FG: " << fgColor.color() << ", bg: " << bgColor.color() << endl;
    m_fgColor = fgColor;
    KoOldColor color = m_fgColor;

    int h = color.H();
    int s = color.S();
    int v = color.V();

    m_HIn->setValue(h);
    m_SIn->setValue(s);
    m_VIn->setValue(v);



    m_HIn->blockSignals(false);
    m_SIn->blockSignals(false);
    m_VIn->blockSignals(false);
    m_colorSlider->blockSignals(false);
    m_xycolorselector->blockSignals(false);
}

void KoUniColorChooser::slotSetColor(const QColor& c)
{
    //kDebug() << "slotFGColorSelected " << c << endl;
    m_fgColor = KoOldColor(c);

    changedFgColor();
}

#include "KoUniColorChooser.moc"
