/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *  Copyright (c) 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_slider_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QBitArray>
#include <QSpacerItem>


#include <klocale.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kglobal.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_color_slider_input.h"
#include <KoColorProfile.h>
#include "kis_debug.h"
#include "kis_signal_compressor.h"
//#include "kis_color_space_selector.h"



KisColorSliderWidget::KisColorSliderWidget(KoColorDisplayRendererInterface *displayRenderer, QWidget* parent, KisCanvas2* canvas, QBitArray SlidersConfigArray)
    : QWidget(parent)
    //, m_colorSpace(0)
    //, m_customColorSpaceSelected(false)
    , m_updateCompressor(new KisSignalCompressor(10, KisSignalCompressor::POSTPONE, this))
    , m_displayRenderer(displayRenderer)
    , m_canvas(canvas)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(1);
    m_updateAllowed = true;
    
    connect(m_updateCompressor, SIGNAL(timeout()), SLOT(updateTimeout()));
    m_configCompressor = new KisSignalCompressor(10, KisSignalCompressor::POSTPONE, this);
    connect(m_configCompressor, SIGNAL(timeout()), SLOT(setConfig()));
    //dbgKrita<<"m_canvas:"<<m_canvas;
        
    m_inputs.clear();
    
    //this first creates and set the sliders.
    //then setslidervisible is called to make them visible and connect them.
    //This way they can also be disabled more easily.
    
    //hsv sliders//
        
    hsvH = new KisHSXColorSliderInput(this, 0, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsvH);
    m_layout->addWidget(hsvH);
    hsvH->setVisible(false);
    
    hsvS = new KisHSXColorSliderInput(this, 1, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsvS);
    m_layout->addWidget(hsvS);
    hsvS->setVisible(false);

    hsvV = new KisHSXColorSliderInput(this, 2, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsvV);
    m_layout->addWidget(hsvV);
    hsvV->setVisible(false);
    
    //hsl sliders//
    
    hslH = new KisHSXColorSliderInput(this, 3, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hslH);
    m_layout->addWidget(hslH);
    hslH->setVisible(false);

    hslS = new KisHSXColorSliderInput(this, 4, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hslS);
    m_layout->addWidget(hslS);
    hslS->setVisible(false);
    
    hslL = new KisHSXColorSliderInput(this, 5, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hslL);
    m_layout->addWidget(hslL);
    hslL->setVisible(false);
    
    
    //hsi sliders//
    
    hsiH = new KisHSXColorSliderInput(this, 6, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsiH);
    m_layout->addWidget(hsiH);
    hsiH->setVisible(false);
    
    hsiS = new KisHSXColorSliderInput(this, 7, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsiS);
    m_layout->addWidget(hsiS);
    hsiS->setVisible(false);
    
    hsiI = new KisHSXColorSliderInput(this, 8, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsiI);
    m_layout->addWidget(hsiI);
    hsiI->setVisible(false);
    
    //hsy'sliders//
    
    hsyH = new KisHSXColorSliderInput(this, 9, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsyH);
    m_layout->addWidget(hsyH);
    hsyH->setVisible(false);
    
    hsyS = new KisHSXColorSliderInput(this, 10, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsyS);
    m_layout->addWidget(hsyS);
    hsyS->setVisible(false);
    
    hsyY = new KisHSXColorSliderInput(this, 11, &m_color, m_displayRenderer, m_canvas);
    m_inputs.append(hsyY);
    m_layout->addWidget(hsyY);
    hsyY->setVisible(false);

    m_layout->addStretch(1);

    setSlidersVisible(SlidersConfigArray);
    

}

KisColorSliderWidget::~KisColorSliderWidget()
{
    //KConfigGroup cfg = KGlobal::config()->group("");
    //cfg.writeEntry("SpecificColorSelector/ShowColorSpaceSelector", m_chkShowColorSpaceSelector->isChecked());

}

void KisColorSliderWidget::update()
{
    if (m_updateAllowed) {
        m_updateCompressor->start();
    }
}

void KisColorSliderWidget::setColor(const KoColor& c)
{
    m_updateAllowed = false;
    m_color.fromKoColor(c);
    emit(updated());
    m_updateAllowed = true;
}

void KisColorSliderWidget::updateTimeout()
{
    emit(colorChanged(m_color));
    
}

void KisColorSliderWidget::setSlidersVisible(QBitArray SlidersConfigArray)
{
    //dbgKrita<<"check2";
    QList<KisColorSliderInput*> visibleInputs;
    
    if (SlidersConfigArray[0]==true) {
        visibleInputs.append(hsvH);
        hsvH->setVisible(true);
        connect(hsvH, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsvH, SLOT(update()));
        connect(hsvH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(hueUpdated(int)), hsvH, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsvH, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsvH, SLOT(toneUpdate(int, int)));
    }
    else {
        hsvH->setVisible(false);
        disconnect(hsvH, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsvH, SLOT(update()));
        disconnect(hsvH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(hueUpdated(int)), hsvH, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsvH, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsvH, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[1]==true) {
        visibleInputs.append(hsvS);
        hsvS->setVisible(true);
        connect(hsvS, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsvS, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsvS, SLOT(hueUpdate(int)));
        connect(hsvS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsvS, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsvS, SLOT(toneUpdate(int, int)));
    }
    else {
        hsvS->setVisible(false);
        disconnect(hsvS, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsvS, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsvS, SLOT(hueUpdate(int)));
        disconnect(hsvS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsvS, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsvS, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[2]==true) {
        visibleInputs.append(hsvV);
        hsvV->setVisible(true);
        connect(hsvV, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsvV, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsvV, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsvV, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsvV, SLOT(toneUpdate(int, int)));
        connect(hsvV, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }
    else {
        hsvV->setVisible(false);
        disconnect(hsvV, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsvV, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsvV, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsvV, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsvV, SLOT(toneUpdate(int, int)));
        disconnect(hsvV, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[3]==true) {
        visibleInputs.append(hslH);
        hslH->setVisible(true);
        connect(hslH, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hslH, SLOT(update()));
        connect(hslH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(hueUpdated(int)), hslH, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hslH, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hslH, SLOT(toneUpdate(int, int)));
    }
    else {
        hslH->setVisible(false);
        disconnect(hslH, SIGNAL(updated()), this,  SLOT(update()));
        disconnect(this, SIGNAL(updated()), hslH, SLOT(update()));
        disconnect(hslH, SIGNAL(hueUpdated(int)), this,  SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(hueUpdated(int)), hslH, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hslH, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hslH, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[4]==true) {
        visibleInputs.append(hslS);
        hslS->setVisible(true);
        connect(hslS, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hslS, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hslS, SLOT(hueUpdate(int)));
        connect(hslS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(satUpdated(int, int)), hslS, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hslS, SLOT(toneUpdate(int, int)));
    }
    else {
        hslS->setVisible(false);
        disconnect(hslS, SIGNAL(updated()), this,  SLOT(update()));
        disconnect(this, SIGNAL(updated()), hslS, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hslS, SLOT(hueUpdate(int)));
        disconnect(hslS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hslS, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hslS, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[5]==true) {
        visibleInputs.append(hslL);
        hslL->setVisible(true);
        connect(hslL, SIGNAL(updated()), this,  SLOT(update()));
        connect(this, SIGNAL(updated()), hslL, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hslL, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hslL, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hslL, SLOT(toneUpdate(int, int)));
        connect(hslL, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }
    else {
        hslL->setVisible(false);
        disconnect(hslL, SIGNAL(updated()), this,  SLOT(update()));
        disconnect(this, SIGNAL(updated()), hslL, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hslL, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hslL, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hslL, SLOT(toneUpdate(int, int)));
        disconnect(hslL, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[6]==true) {
        visibleInputs.append(hsiH);
        hsiH->setVisible(true);
        connect(hsiH, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsiH, SLOT(update()));
        connect(hsiH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(hueUpdated(int)), hsiH, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsiH, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsiH, SLOT(toneUpdate(int, int)));
    }
    else {
        hsiH->setVisible(false);
        disconnect(hsiH, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsiH, SLOT(update()));
        disconnect(hsiH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(hueUpdated(int)), hsiH, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsiH, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsiH, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[7]==true) {
        visibleInputs.append(hsiS);
        hsiS->setVisible(true);
        connect(hsiS, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsiS, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsiS, SLOT(hueUpdate(int)));
        connect(hsiS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsiS, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsiS, SLOT(toneUpdate(int, int)));
    }
    else {
        hsiS->setVisible(false);
        disconnect(hsiS, SIGNAL(updated()), this,  SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsiS, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsiS, SLOT(hueUpdate(int)));
        disconnect(hsiS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsiS, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsiS, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[8]==true) {
        visibleInputs.append(hsiI);
        hsiI->setVisible(true);
        connect(hsiI, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsiI, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsiI, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsiI, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsiI, SLOT(toneUpdate(int, int)));
        connect(hsiI, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }
    else {
        hsiI->setVisible(false);
        disconnect(hsiI, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsiI, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsiI, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsiI, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsiI, SLOT(toneUpdate(int, int)));
        disconnect(hsiI, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[9]==true) {
        visibleInputs.append(hsyH);
        hsyH->setVisible(true);
        connect(hsyH, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsyH, SLOT(update()));
        connect(hsyH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(hueUpdated(int)), hsyH, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsyH, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsyH, SLOT(toneUpdate(int, int)));
    }
    else {
        hsyH->setVisible(false);
        disconnect(hsyH, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsyH, SLOT(update()));
        disconnect(hsyH, SIGNAL(hueUpdated(int)), this, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(hueUpdated(int)), hsyH, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsyH, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsyH, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[10]==true) {
        visibleInputs.append(hsyS);
        hsyS->setVisible(true);
        connect(hsyS, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsyS, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsyS, SLOT(hueUpdate(int)));
        connect(hsyS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsyS, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsyS, SLOT(toneUpdate(int, int)));
    }
    else {
        hsyS->setVisible(false);
        disconnect(hsyS, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsyS, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsyS, SLOT(hueUpdate(int)));
        disconnect(hsyS, SIGNAL(satUpdated(int, int)), this, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsyS, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsyS, SLOT(toneUpdate(int, int)));
    }

    if (SlidersConfigArray[11]==true) {
        visibleInputs.append(hsyY);
        hsyY->setVisible(true);
        connect(hsyY, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(updated()), hsyY, SLOT(update()));
        connect(this, SIGNAL(hueUpdated(int)), hsyY, SLOT(hueUpdate(int)));
        connect(this, SIGNAL(satUpdated(int, int)), hsyY, SLOT(satUpdate(int, int)));
        connect(this, SIGNAL(toneUpdated(int, int)), hsyY, SLOT(toneUpdate(int, int)));
        connect(hsyY, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }
    else {
        hsyY->setVisible(false);
        disconnect(hsyY, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(updated()), hsyY, SLOT(update()));
        disconnect(this, SIGNAL(hueUpdated(int)), hsyY, SLOT(hueUpdate(int)));
        disconnect(this, SIGNAL(satUpdated(int, int)), hsyY, SLOT(satUpdate(int, int)));
        disconnect(this, SIGNAL(toneUpdated(int, int)), hsyY, SLOT(toneUpdate(int, int)));
        disconnect(hsyY, SIGNAL(toneUpdated(int, int)), this, SLOT(toneUpdate(int, int)));
    }

    QList<QLabel*> labels;
    int labelWidth = 0;

    Q_FOREACH (KisColorSliderInput* input, visibleInputs) {
        Q_FOREACH (QLabel* label, input->findChildren<QLabel*>()) {
            labels.append(label);
            labelWidth = qMax(labelWidth, label->sizeHint().width());
        }
    }

    Q_FOREACH (QLabel *label, labels) {
        label->setMinimumWidth(labelWidth);
    }


    updateTimeout();
}
void KisColorSliderWidget::slotConfigChanged()
{
    if (m_updateAllowed) {
        m_configCompressor->start();
    }
}

void KisColorSliderWidget::setConfig()
{
    //QTimer::singleShot(1, this, SLOT(update()));//need to wait a bit before accessing the config.

    QBitArray m_SlidersConfigArray(12);
    //dbgKrita<<"check";
    KConfigGroup cfg = KGlobal::config()->group("hsxColorSlider");

    m_SlidersConfigArray[0] =cfg.readEntry("hsvH", false);
    m_SlidersConfigArray[1] =cfg.readEntry("hsvS", false);
    m_SlidersConfigArray[2] =cfg.readEntry("hsvV", false);
    
    m_SlidersConfigArray[3] =cfg.readEntry("hslH", true);
    m_SlidersConfigArray[4] =cfg.readEntry("hslS", true);
    m_SlidersConfigArray[5] =cfg.readEntry("hslL", true);
    
    m_SlidersConfigArray[6] =cfg.readEntry("hsiH", false);
    m_SlidersConfigArray[7] =cfg.readEntry("hsiS", false);
    m_SlidersConfigArray[8] =cfg.readEntry("hsiI", false);
    
    m_SlidersConfigArray[9] =cfg.readEntry("hsyH", false);
    m_SlidersConfigArray[10]=cfg.readEntry("hsyS", false);
    m_SlidersConfigArray[11]=cfg.readEntry("hsyY", false);
    
    setSlidersVisible(m_SlidersConfigArray);
    
}

void KisColorSliderWidget::hueUpdate(int h)
{
    emit(hueUpdated(h));
}

void KisColorSliderWidget::satUpdate(int s, int type)
{
    emit(satUpdated(s, type));
}
void KisColorSliderWidget::toneUpdate(int l, int type)
{
    emit(toneUpdated(l, type));
}

#include "moc_kis_color_slider_widget.cpp"