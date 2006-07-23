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
#include "KoColorSpaceRegistry.h"
#include "KoXYColorSelector.h"
#include "KoColorSlider.h"
#include "KoColorPatch.h"

#include "KoUniColorChooser.h"

KoUniColorChooser::KoUniColorChooser(QWidget *parent, bool opacitySlider) : super(parent), m_showOpacitySlider(opacitySlider)
{
    QGridLayout *mGrid = new QGridLayout;
    QGridLayout *mGrowGrid = new QGridLayout;

    m_xycolorselector = new KoXYColorSelector(rgbColorSpace(), this);
    m_xycolorselector->setFixedSize(137, 137);

    m_colorSlider = new KoColorSlider(Qt::Vertical, this);
    m_colorSlider->setFixedSize(25, 118);

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
    m_CLabel = new QLabel("C:", this);
    m_CLabel->setFixedSize(10, 18);
    m_MLabel = new QLabel("M:", this);
    m_MLabel->setFixedSize(10, 18);
    m_YLabel = new QLabel("Y:", this);
    m_YLabel->setFixedSize(10, 18);
    m_KLabel = new QLabel("K:", this);
    m_KLabel->setFixedSize(10, 18);
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

    m_CIn = new QSpinBox(this);
    m_CIn->setMinimum(0);
    m_CIn->setMaximum(255);
    m_CIn->setSingleStep(1);
    m_CIn->setFixedSize(40, 18);
    m_CIn->setFocusPolicy( Qt::ClickFocus );
    m_CIn->setToolTip( i18n( "Cyan" ) );

    m_MIn = new QSpinBox(this);
    m_MIn->setMinimum(0);
    m_MIn->setMaximum(255);
    m_MIn->setSingleStep(1);
    m_MIn->setFixedSize(40, 18);
    m_MIn->setFocusPolicy( Qt::ClickFocus );
    m_MIn->setToolTip( i18n( "Magenta" ) );

    m_YIn = new QSpinBox(this);
    m_YIn->setMinimum(0);
    m_YIn->setMaximum(255);
    m_YIn->setSingleStep(1);
    m_YIn->setFixedSize(40, 18);
    m_YIn->setFocusPolicy( Qt::ClickFocus );
    m_YIn->setToolTip( i18n( "Yellow" ) );

    m_KIn = new QSpinBox(this);
    m_KIn->setMinimum(0);
    m_KIn->setMaximum(255);
    m_KIn->setSingleStep(1);
    m_KIn->setFixedSize(40, 18);
    m_KIn->setFocusPolicy( Qt::ClickFocus );
    m_KIn->setToolTip( i18n( "Black" ) );

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

    mGrid->setSpacing(1);
    mGrid->setMargin(0);

    mGrid->addWidget(m_xycolorselector, 0, 0, -1, 1, Qt::AlignTop);

    mGrid->addWidget(m_colorpatch, 0, 1, 1, 4, Qt::AlignCenter);

    mGrid->addWidget(m_colorSlider, 1, 1, -1, 1, Qt::AlignTop);

    mGrid->addWidget(m_HRB, 1, 2, Qt::AlignCenter);
    mGrid->addWidget(m_SRB, 2, 2, Qt::AlignCenter);
    mGrid->addWidget(m_VRB, 3, 2, Qt::AlignCenter);
    mGrid->addWidget(m_RRB, 5, 2, Qt::AlignCenter);
    mGrid->addWidget(m_GRB, 6, 2, Qt::AlignCenter);
    mGrid->addWidget(m_BRB, 7, 2, Qt::AlignCenter);
    mGrid->addWidget(m_LRB, 5, 6, Qt::AlignCenter);
    mGrid->addWidget(m_aRB, 6, 6, Qt::AlignCenter);
    mGrid->addWidget(m_bRB, 7, 6, Qt::AlignCenter);

    mGrid->addWidget(m_HLabel, 1, 3, Qt::AlignTop);
    mGrid->addWidget(m_SLabel, 2, 3, Qt::AlignTop);
    mGrid->addWidget(m_VLabel, 3, 3, Qt::AlignTop);
    mGrid->addWidget(m_RLabel, 5, 3, Qt::AlignTop);
    mGrid->addWidget(m_GLabel, 6, 3, Qt::AlignTop);
    mGrid->addWidget(m_BLabel, 7, 3, Qt::AlignTop);
    mGrid->addWidget(m_CLabel, 0, 7, Qt::AlignTop);
    mGrid->addWidget(m_MLabel, 1, 7, Qt::AlignTop);
    mGrid->addWidget(m_YLabel, 2, 7, Qt::AlignTop);
    mGrid->addWidget(m_KLabel, 3, 7, Qt::AlignTop);
    mGrid->addWidget(m_LLabel, 5, 7, Qt::AlignTop);
    mGrid->addWidget(m_aLabel, 6, 7, Qt::AlignTop);
    mGrid->addWidget(m_bLabel, 7, 7, Qt::AlignTop);

    mGrid->addWidget(m_HIn, 1, 4, Qt::AlignTop);
    mGrid->addWidget(m_SIn, 2, 4, Qt::AlignTop);
    mGrid->addWidget(m_VIn, 3, 4, Qt::AlignTop);
    mGrid->addWidget(m_RIn, 5, 4, Qt::AlignTop);
    mGrid->addWidget(m_GIn, 6, 4, Qt::AlignTop);
    mGrid->addWidget(m_BIn, 7, 4, Qt::AlignTop);
    mGrid->addWidget(m_CIn, 0, 8, Qt::AlignTop);
    mGrid->addWidget(m_MIn, 1, 8, Qt::AlignTop);
    mGrid->addWidget(m_YIn, 2, 8, Qt::AlignTop);
    mGrid->addWidget(m_KIn, 3, 8, Qt::AlignTop);
    mGrid->addWidget(m_LIn, 5, 8, Qt::AlignTop);
    mGrid->addWidget(m_aIn, 6, 8, Qt::AlignTop);
    mGrid->addWidget(m_bIn, 7, 8, Qt::AlignTop);

    if(m_showOpacitySlider)
    {
        m_opacityLabel = new QLabel(i18n( "Opacity:" ), this);

        m_opacitySlider = new KoColorSlider(Qt::Horizontal, this);
        m_opacitySlider->setFixedSize(100, 25);
        m_opacitySlider->setRange(0, 100);

        m_opacityIn = new QSpinBox(this);
        m_opacityIn->setRange(0, 100);
        m_opacityIn->setSingleStep(1);
        m_opacityIn->setFixedSize(40, 18);
        m_opacityIn->setFocusPolicy( Qt::ClickFocus );

        mGrid->addItem( new QSpacerItem( 4, 4, QSizePolicy::Fixed, QSizePolicy::Fixed), 8, 5 );

        mGrid->addWidget(m_opacityLabel, 9, 2, Qt::AlignTop);
        mGrid->addWidget(m_opacitySlider, 9, 3, 1, 4, Qt::AlignTop);
        mGrid->addWidget(m_opacityIn, 9, 8, Qt::AlignTop);

        connect(m_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotOpacityChanged(int)));
        connect(m_opacityIn, SIGNAL(valueChanged(int)), this, SLOT(slotOpacityChanged(int)));
    }

    mGrid->addItem( new QSpacerItem( 4, 4, QSizePolicy::Fixed, QSizePolicy::Fixed), 4, 5 );
    //mGrid->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding ), 8, 9 );

    /* connect spin box */
    connect(m_HIn, SIGNAL(valueChanged(int)), this, SLOT(slotHSVChanged()));
    connect(m_SIn, SIGNAL(valueChanged(int)), this, SLOT(slotHSVChanged()));
    connect(m_VIn, SIGNAL(valueChanged(int)), this, SLOT(slotHSVChanged()));
    connect(m_RIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));
    connect(m_GIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));
    connect(m_BIn, SIGNAL(valueChanged(int)), this, SLOT(slotRGBChanged()));

    /* connect radio buttons */
    connect(m_HRB, SIGNAL(toggled(bool)), this, SLOT(slotHSelected(bool)));
    connect(m_SRB, SIGNAL(toggled(bool)), this, SLOT(slotSSelected(bool)));
    connect(m_VRB, SIGNAL(toggled(bool)), this, SLOT(slotVSelected(bool)));
    connect(m_RRB, SIGNAL(toggled(bool)), this, SLOT(slotRSelected(bool)));
    connect(m_GRB, SIGNAL(toggled(bool)), this, SLOT(slotGSelected(bool)));
    connect(m_BRB, SIGNAL(toggled(bool)), this, SLOT(slotBSelected(bool)));

    /* connect slider */
    connect(m_xycolorselector, SIGNAL(valueChanged(int,int)), this, SLOT(slotXYChanged(int,int)));

    /* connect sxy */
    connect(m_colorSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));

    m_RRB->setChecked(true);

    mGrowGrid->addLayout(mGrid, 0, 0);
    mGrowGrid->setSpacing(0);
    mGrowGrid->setMargin(0);
    mGrowGrid->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding ), 1, 1 );

    setLayout(mGrowGrid);

    updateValues();
}

KoColor KoUniColorChooser::color()
{
    return m_currentColor;
}

void KoUniColorChooser::setColor(const KoColor & c)
{
    m_currentColor = c;
    updateValues();
    updateSelectorsCurrent();
    m_colorpatch->setColor(m_currentColor);
}

void KoUniColorChooser::slotHSVChanged()
{
    quint8 data[4];
    HSVtoRGB(m_HIn->value(), m_SIn->value(), m_VIn->value(), data+2, data+1, data);
    data[3] = m_currentColor.colorSpace()->getAlpha(m_currentColor.data());
    m_currentColor.setColor(data, rgbColorSpace());
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotRGBChanged()
{
    quint8 data[4];
    data[2] = m_RIn->value();
    data[1] = m_GIn->value();
    data[0] = m_BIn->value();
    data[3] = m_currentColor.colorSpace()->getAlpha(m_currentColor.data());
    m_currentColor.setColor(data, rgbColorSpace());
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotSliderChanged(int v)
{
    quint8 data[4];
    data[3] = m_currentColor.colorSpace()->getAlpha(m_currentColor.data());
    switch(m_activeChannel)
    {
        case CHANNEL_H:
            HSVtoRGB(v, m_SIn->value(), m_VIn->value(), data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_S:
            HSVtoRGB(m_HIn->value(), v, m_VIn->value(), data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_V:
            HSVtoRGB(m_HIn->value(), m_SIn->value(), v, data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_R:
            data[2] = v;
            data[1] = m_GIn->value();
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_G:
            data[2] = m_RIn->value();
            data[1] = v;
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_B:
            data[2] = m_RIn->value();
            data[1] = m_GIn->value();
            data[0] = v;
            m_currentColor.setColor(data, rgbColorSpace());
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
    data[3] = m_currentColor.colorSpace()->getAlpha(m_currentColor.data());
    switch(m_activeChannel)
    {
        case CHANNEL_H:
            HSVtoRGB(m_HIn->value(), u, v, data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_S:
            HSVtoRGB(u, m_SIn->value(), v, data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_V:
            HSVtoRGB(u, v, m_VIn->value(), data+2, data+1, data);
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_R:
            data[2] = m_RIn->value();
            data[1] = v;
            data[0] = u;
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_G:
            data[2] = v;
            data[1] = m_GIn->value();
            data[0] = u;
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_B:
            data[2] = u;
            data[1] = v;
            data[0] = m_BIn->value();
            m_currentColor.setColor(data, rgbColorSpace());
            break;
        case CHANNEL_L:
            break;
        case CHANNEL_a:
            break;
        case CHANNEL_b:
            break;
    }
    updateValues();
    updateSelectorsCurrent();
    announceColor();
}

void KoUniColorChooser::slotOpacityChanged(int o)
{
    quint8 opacity = o * OPACITY_OPAQUE / 100;
    m_currentColor.colorSpace()->setAlpha(m_currentColor.data(), opacity, 1);
    updateValues();
    announceColor();
}

void KoUniColorChooser::slotHSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_H;
        updateSelectorsR();
    }
}

void KoUniColorChooser::slotSSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_S;
        updateSelectorsG();
    }
}

void KoUniColorChooser::slotVSelected(bool s)
{
    if(s)
    {
        m_activeChannel = CHANNEL_V;
        updateSelectorsB();
    }
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
    KoColor topleft(data, rgbColorSpace());
    data[1] = 255;
    data[0] = 255;
    KoColor topright(data, rgbColorSpace());
    data[1] = 0;
    data[0] = 0;
    KoColor bottomleft(data, rgbColorSpace());
    data[1] = 0;
    data[0] = 255;
    KoColor bottomright(data, rgbColorSpace());
    
    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = 0;
    data[1] = m_GIn->value();
    data[0] = m_BIn->value();
    KoColor mincolor(data, rgbColorSpace());
    data[2] = 255;
    KoColor maxcolor(data, rgbColorSpace());

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
    KoColor topleft(data, rgbColorSpace());
    data[2] = 255;
    data[0] = 255;
    KoColor topright(data, rgbColorSpace());
    data[2] = 0;
    data[0] = 0;
    KoColor bottomleft(data, rgbColorSpace());
    data[2] = 0;
    data[0] = 255;
    KoColor bottomright(data, rgbColorSpace());

    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = m_RIn->value();
    data[1] = 0;
    data[0] = m_BIn->value();
    KoColor mincolor(data, rgbColorSpace());
    data[1] = 255;
    KoColor maxcolor(data, rgbColorSpace());

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
    KoColor topleft(data, rgbColorSpace());
    data[2] = 255;
    data[1] = 255;
    KoColor topright(data, rgbColorSpace());
    data[2] = 0;
    data[1] = 0;
    KoColor bottomleft(data, rgbColorSpace());
    data[2] = 255;
    data[1] = 0;
    KoColor bottomright(data, rgbColorSpace());
    
    m_xycolorselector->setColors(topleft,topright,bottomleft,bottomright);

    data[2] = m_RIn->value();
    data[1] = m_GIn->value();
    data[0] = 0;
    KoColor mincolor(data, rgbColorSpace());
    data[0] = 255;
    KoColor maxcolor(data, rgbColorSpace());

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

    emit sigColorChanged(m_currentColor);
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
    m_CIn->blockSignals(true);
    m_MIn->blockSignals(true);
    m_YIn->blockSignals(true);
    m_KIn->blockSignals(true);
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
    tmpColor.convertTo(rgbColorSpace());
    m_RIn->setValue(tmpColor.data()[2]);
    m_GIn->setValue(tmpColor.data()[1]);
    m_BIn->setValue(tmpColor.data()[0]);

    if(m_showOpacitySlider)
    {
        m_opacitySlider->blockSignals(true);
        m_opacityIn->blockSignals(true);

        KoColor minColor = tmpColor;
        tmpColor.colorSpace()->setAlpha(minColor.data(), 0, 1);
        KoColor maxColor = tmpColor;
        tmpColor.colorSpace()->setAlpha(maxColor.data(), 255, 1);

        m_opacitySlider->setColors(minColor, maxColor);
        m_opacitySlider->setValue(tmpColor.data()[3]* 100 / OPACITY_OPAQUE);
        m_opacityIn->setValue(tmpColor.data()[3]* 100 / OPACITY_OPAQUE);

        m_opacityIn->blockSignals(false);
        m_opacitySlider->blockSignals(false);
    }

    tmpColor = m_currentColor;
    tmpColor.convertTo(labColorSpace());
    m_LIn->setValue(((quint16 *)tmpColor.data())[0]/(256*256/100));
    m_aIn->setValue(((quint16 *)tmpColor.data())[1]/256);
    m_bIn->setValue(((quint16 *)tmpColor.data())[2]/256);

    tmpColor = m_currentColor;
    tmpColor.convertTo(cmykColorSpace());
    m_CIn->setValue((tmpColor.data()[0]*100)/255);
    m_MIn->setValue((tmpColor.data()[1]*100/255));
    m_YIn->setValue((tmpColor.data()[2]*100)/255);
    m_KIn->setValue((tmpColor.data()[3]*100)/255);

    m_HIn->blockSignals(false);
    m_SIn->blockSignals(false);
    m_VIn->blockSignals(false);
    m_RIn->blockSignals(false);
    m_GIn->blockSignals(false);
    m_BIn->blockSignals(false);
    m_CIn->blockSignals(false);
    m_MIn->blockSignals(false);
    m_YIn->blockSignals(false);
    m_KIn->blockSignals(false);
    m_LIn->blockSignals(false);
    m_aIn->blockSignals(false);
    m_bIn->blockSignals(false);
}

KoColorSpace *KoUniColorChooser::rgbColorSpace()
{
    return KoColorSpaceRegistry::instance()->rgb8();
}

KoColorSpace *KoUniColorChooser::labColorSpace()
{
    return KoColorSpaceRegistry::instance()->colorSpace(KoID("LABA",0),"");
}

KoColorSpace *KoUniColorChooser::cmykColorSpace()
{
    return KoColorSpaceRegistry::instance()->colorSpace(KoID("CMYK",0),"");
}

void KoUniColorChooser::RGBtoHSV(int R, int G, int B, int *H, int *S, int *V)
{
  unsigned int max = R;
  unsigned int min = R;
  unsigned char maxValue = 0; // r = 0, g = 1, b = 2

  // find maximum and minimum RGB values
  if(static_cast<unsigned int>(G) > max)
  {
    max = G;
    maxValue = 1;
  }
  if(static_cast<unsigned int>(B) > max)
  {
    max = B;
    maxValue = 2;
  }

  if(static_cast<unsigned int>(G) < min)
    min = G;
  if(static_cast<unsigned int>(B) < min )
    min = B;

  int delta = max - min;
  *V = max; // value
  *S = max ? (510 * delta + max) / ( 2 * max) : 0; // saturation

  // calc hue
  if(*S == 0)
    *H = -1; // undefined hue
  else
  {
    switch(maxValue)
    {
    case 0:  // red
      if(G >= B)
        *H = (120 * (G - B) + delta) / (2 * delta);
      else
        *H = (120 * (G - B + delta) + delta) / (2 * delta) + 300;
      break;
    case 1:  // green
      if(B > R)
        *H = 120 + (120 * (B - R) + delta) / (2 * delta);
      else
        *H = 60 + (120 * (B - R + delta) + delta) / (2 * delta);
      break;
    case 2:  // blue
      if(R > G)
        *H = 240 + (120 * (R - G) + delta) / (2 * delta);
      else
        *H = 180 + (120 * (R - G + delta) + delta) / (2 * delta);
      break;
    }
  }
}

void KoUniColorChooser::HSVtoRGB(int H, int S, int V, quint8 *R, quint8 *G, quint8 *B)
{
  *R = *G = *B = V;

  if(S != 0 && H != -1) // chromatic
  {
    if(H >= 360) // angle > 360
      H %= 360;

    unsigned int f = H % 60;
    H /= 60;
    unsigned int p = static_cast<unsigned int>(2*V*(255-S)+255)/510;
    unsigned int q, t;

    if(H & 1)
    {
      q = static_cast<unsigned int>(2 * V * (15300 - S * f) + 15300) / 30600;
      switch(H)
      {
      case 1:
        *R = static_cast<quint8>(q);
	*G = static_cast<quint8>(V);
	*B = static_cast<quint8>(p);
	break;
      case 3:
        *R = static_cast<quint8>(p);
	*G = static_cast<quint8>(q);
	*B = static_cast<quint8>(V);
	break;
      case 5:
        *R = static_cast<quint8>(V);
	*G = static_cast<quint8>(p);
	*B = static_cast<quint8>(q);
	break;
      }
    }
    else
    {
      t = static_cast<unsigned int>(2 * V * (15300 - (S * (60 - f))) + 15300) / 30600;
      switch(H)
      {
      case 0:
        *R = static_cast<quint8>(V);
        *G = static_cast<quint8>(t);
        *B = static_cast<quint8>(p);
        break;
      case 2:
        *R = static_cast<quint8>(p);
        *G = static_cast<quint8>(V);
        *B = static_cast<quint8>(t);
        break;
      case 4:
        *R = static_cast<quint8>(t);
        *G = static_cast<quint8>(p);
        *B = static_cast<quint8>(V);
        break;
      }
    }
  }
}

#include "KoUniColorChooser.moc"
