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

#include <kdebug.h>
#include <klocale.h>

#include "KoColor.h"
#include "KoColorSpaceFactoryRegistry.h"
#include "KoXYColorSelector.h"
#include "KoColorSlider.h"
#include "KoColorPatch.h"

#include "KoUniColorChooser.h"

KoUniColorChooser::KoUniColorChooser(KoColorSpaceFactoryRegistry* csFactoryRegistry, QWidget *parent) : super(parent)
{
    m_csFactoryRegistry = csFactoryRegistry;

    QGridLayout *mGrid = new QGridLayout;

    m_xycolorselector = new KoXYColorSelector(csFactoryRegistry->getRGB8(), this);
    m_xycolorselector->setFixedSize(120, 120);

    m_colorSlider = new KoColorSlider(csFactoryRegistry->getRGB8(), Qt::Vertical, this);
    m_colorSlider->setFixedSize(25, 100);

    m_colorpatch = new KoColorPatch(this);
    m_colorpatch->setFixedSize(18, 18);

    /* setup channel labels */
    m_HLabel = new QLabel("H:", this);
    m_HLabel->setFixedSize(10, 18);
    m_HLabel->setEnabled(false);
    m_SLabel = new QLabel("S:", this);
    m_SLabel->setFixedSize(10, 18);
    m_SLabel->setEnabled(false);
    m_VLabel = new QLabel("V:", this);
    m_VLabel->setFixedSize(10, 18);
    m_VLabel->setEnabled(false);
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
    m_HRB->setEnabled(false);
    m_SRB = new QRadioButton(this);
    m_SRB->setEnabled(false);
    m_VRB = new QRadioButton(this);
    m_VRB->setEnabled(false);
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
    m_HIn->setEnabled(false);

    m_SIn = new QSpinBox(this);
    m_SIn->setMinimum(0);
    m_SIn->setMaximum(255);
    m_SIn->setSingleStep(1);
    m_SIn->setFixedSize(40, 18);
    m_SIn->setFocusPolicy( Qt::ClickFocus );
    m_SIn->setToolTip( i18n( "Saturation" ) );
    m_SIn->setEnabled(false);

    m_VIn = new QSpinBox(this);
    m_VIn->setMinimum(0);
    m_VIn->setMaximum(255);
    m_VIn->setSingleStep(1);
    m_VIn->setFixedSize(40, 18);
    m_VIn->setFocusPolicy( Qt::ClickFocus );
    m_VIn->setToolTip( i18n( "Value (brightness)" ) );
    m_VIn->setEnabled(false);

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

    /* connect spin box */
    connect(m_HIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
    connect(m_SIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
    connect(m_VIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
    connect(m_RIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));
    connect(m_GIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));
    connect(m_BIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));

    /* connect radio buttons */
    connect(m_RRB, SIGNAL(toggled(bool)), this, SLOT(slotRSelected(bool)));
    connect(m_GRB, SIGNAL(toggled(bool)), this, SLOT(slotGSelected(bool)));
    connect(m_BRB, SIGNAL(toggled(bool)), this, SLOT(slotBSelected(bool)));

    /* connect slider */
    connect(m_xycolorselector, SIGNAL(valueChanged(int,int)), this, SLOT(slotXYChanged(int,int)));

    /* connect sxy */
    connect(m_colorSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));

    m_RRB->setChecked(true);
    
    setLayout(mGrid);
}

void KoUniColorChooser::slotHChanged(int )
{
}

void KoUniColorChooser::slotSChanged(int )
{
}

void KoUniColorChooser::slotVChanged(int )
{
}

void KoUniColorChooser::slotRGBChanged()
{
    quint8 data[4];
    data[2] = m_RIn->value();
    data[1] = m_GIn->value();
    data[0] = m_BIn->value();
    m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotSliderChanged(int v)
{
    quint8 data[4];
    switch(m_activeChannel)
    {
        case CHANNEL_H:
            //slotRSelected(true);
            break;
        case CHANNEL_S:
            //slotGSelected(true);
            break;
        case CHANNEL_V:
            //slotBSelected(true);
            break;
        case CHANNEL_R:
            data[2] = v;
            data[1] = m_GIn->value();
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_G:
            data[2] = m_RIn->value();
            data[1] = v;
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_B:
            data[2] = m_RIn->value();
            data[1] = m_GIn->value();
            data[0] = v;
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_L:
            //slotRSelected(true);
            break;
        case CHANNEL_a:
            //slotGSelected(true);
            break;
        case CHANNEL_b:
            //slotBSelected(true);
            break;
    }
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotXYChanged(int u, int v)
{
    quint8 data[4];
    switch(m_activeChannel)
    {
        case CHANNEL_H:
            //slotRSelected(true);
            break;
        case CHANNEL_S:
            //slotGSelected(true);
            break;
        case CHANNEL_V:
            //slotBSelected(true);
            break;
        case CHANNEL_R:
            data[2] = m_RIn->value();
            data[1] = v;
            data[0] = u;
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_G:
            data[2] = v;
            data[1] = m_GIn->value();
            data[0] = u;
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_B:
            data[2] = u;
            data[1] = v;
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, m_csFactoryRegistry->getRGB8());
            break;
        case CHANNEL_L:
            //slotRSelected(true);
            break;
        case CHANNEL_a:
            //slotGSelected(true);
            break;
        case CHANNEL_b:
            //slotBSelected(true);
            break;
    }
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotRSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_R;
        updateSelectorsR();
    }
}

void KoUniColorChooser::slotGSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_G;
        updateSelectorsG();
    }
}

void KoUniColorChooser::slotBSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_B;
        updateSelectorsB();
    }
}

void KoUniColorChooser::updateSelectorsR()
{
    //kDebug() << "R selected" << endl;

    quint8 data[4];
    data[2] = m_RIn->value();
    data[1] = 255;
    data[0] = 0;
    data[3] = 255;
    KoColor topleft(data, m_csFactoryRegistry->getRGB8());
    data[1] = 255;
    data[0] = 255;
    KoColor topright(data, m_csFactoryRegistry->getRGB8());
    data[1] = 0;
    data[0] = 0;
    KoColor bottomleft(data, m_csFactoryRegistry->getRGB8());
    data[1] = 0;
    data[0] = 255;
    KoColor bottomright(data, m_csFactoryRegistry->getRGB8());
    
    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = 0;
    data[1] = m_GIn->value();
    data[0] = m_BIn->value();
    KoColor mincolor(data,m_csFactoryRegistry->getRGB8());
    data[2] = 255;
    KoColor maxcolor(data,m_csFactoryRegistry->getRGB8());

    m_colorSlider->setColors(mincolor, maxcolor);

    m_xycolorselector->blockSignals(true);
    m_colorSlider->blockSignals(true);
    m_xycolorselector->setValues(m_BIn->value(), m_GIn->value());
    m_colorSlider->setValue(m_RIn->value());
    m_xycolorselector->blockSignals(false);
    m_colorSlider->blockSignals(false);
}

void KoUniColorChooser::updateSelectorsG()
{
    //kDebug() << "G selected" << endl;

    quint8 data[4];
    data[2] = 255;
    data[1] = m_GIn->value();
    data[0] = 0;
    data[3] = 255;
    KoColor topleft(data,m_csFactoryRegistry->getRGB8());
    data[2] = 255;
    data[0] = 255;
    KoColor topright(data,m_csFactoryRegistry->getRGB8());
    data[2] = 0;
    data[0] = 0;
    KoColor bottomleft(data,m_csFactoryRegistry->getRGB8());
    data[2] = 0;
    data[0] = 255;
    KoColor bottomright(data,m_csFactoryRegistry->getRGB8());

    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = m_RIn->value();
    data[1] = 0;
    data[0] = m_BIn->value();
    KoColor mincolor(data,m_csFactoryRegistry->getRGB8());
    data[1] = 255;
    KoColor maxcolor(data,m_csFactoryRegistry->getRGB8());

    m_colorSlider->setColors(mincolor, maxcolor);

    m_xycolorselector->blockSignals(true);
    m_colorSlider->blockSignals(true);
    m_xycolorselector->setValues(m_BIn->value(), m_RIn->value());
    m_colorSlider->setValue(m_GIn->value());
    m_xycolorselector->blockSignals(false);
    m_colorSlider->blockSignals(false);
}

void KoUniColorChooser::updateSelectorsB()
{
    //kDebug() << "B selected" << endl;

    quint8 data[4];
    data[2] = 0;
    data[1] = 255;
    data[0] = m_BIn->value();
    data[3] = 255;
    KoColor topleft(data,m_csFactoryRegistry->getRGB8());
    data[2] = 255;
    data[1] = 255;
    KoColor topright(data,m_csFactoryRegistry->getRGB8());
    data[2] = 0;
    data[1] = 0;
    KoColor bottomleft(data,m_csFactoryRegistry->getRGB8());
    data[2] = 255;
    data[1] = 0;
    KoColor bottomright(data,m_csFactoryRegistry->getRGB8());
    
    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = m_RIn->value();
    data[1] = m_GIn->value();
    data[0] = 0;
    KoColor mincolor(data,m_csFactoryRegistry->getRGB8());
    data[0] = 255;
    KoColor maxcolor(data,m_csFactoryRegistry->getRGB8());

    m_colorSlider->setColors(mincolor, maxcolor);

    m_xycolorselector->blockSignals(true);
    m_colorSlider->blockSignals(true);
    m_xycolorselector->setValues(m_RIn->value(), m_GIn->value());
    m_colorSlider->setValue(m_BIn->value());
    m_xycolorselector->blockSignals(false);
    m_colorSlider->blockSignals(false);
}

void KoUniColorChooser::updateSelectorsCurrent()
{
    switch(m_activeChannel)
    {
        case CHANNEL_H:
            //slotRSelected(true);
            break;
        case CHANNEL_S:
            //slotGSelected(true);
            break;
        case CHANNEL_V:
            //slotBSelected(true);
            break;
        case CHANNEL_R:
            updateSelectorsR();
            break;
        case CHANNEL_G:
            updateSelectorsG();
            break;
        case CHANNEL_B:
            updateSelectorsB();
            break;
        case CHANNEL_L:
            //updateSelectorsL();
            break;
        case CHANNEL_a:
            //updateSelectorsa();
            break;
        case CHANNEL_b:
            //updateSelectorsb();
            break;
    }
}

void KoUniColorChooser::announceColor()
{
    m_colorpatch->setColor(m_currentColor);

    QColor c;
    m_currentColor.toQColor(&c);
    emit sigColorChanged(c);
}

void KoUniColorChooser::updateValues()
{
    KoColor tmpColor;
    m_HIn->blockSignals(true);
    m_SIn->blockSignals(true);
    m_VIn->blockSignals(true);
    m_RIn->blockSignals(true);
    m_GIn->blockSignals(true);
    m_BIn->blockSignals(true);
    m_LIn->blockSignals(true);
    m_aIn->blockSignals(true);
    m_bIn->blockSignals(true);

/*
    KoOldColor color = m_fgColor;

    int h = color.H();
    int s = color.S();
    int v = color.V();

    m_HIn->setValue(h);
    m_SIn->setValue(s);
    m_VIn->setValue(v);
*/
    tmpColor = m_currentColor;
    tmpColor.convertTo(m_csFactoryRegistry->getRGB8());
    m_RIn->setValue(tmpColor.data()[2]);
    m_GIn->setValue(tmpColor.data()[1]);
    m_BIn->setValue(tmpColor.data()[0]);

/*    m_LIn->setValue(h);
    m_aIn->setValue(s);
    m_bIn->setValue(v);
*/
    m_HIn->blockSignals(false);
    m_SIn->blockSignals(false);
    m_VIn->blockSignals(false);
    m_RIn->blockSignals(false);
    m_GIn->blockSignals(false);
    m_BIn->blockSignals(false);
    m_LIn->blockSignals(false);
    m_aIn->blockSignals(false);
    m_bIn->blockSignals(false);
}

#include "KoUniColorChooser.moc"
