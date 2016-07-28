/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#include "kis_spinbox_color_selector.h"
#include <QFormLayout>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "kis_signal_compressor.h"

#include "KoChannelInfo.h"
#include "KoColorSpaceTraits.h"
#include "KoColorSpaceMaths.h"
#include "KoColorSpaceRegistry.h"

struct KisSpinboxColorSelector::Private
{
    bool allowUpdates = false;
    QList <QSpinBox*> spinBoxList;
    QList <QDoubleSpinBox*> doubleSpinBoxList;
    KoColor color;
    const KoColorSpace *cs;
    KisSignalCompressor *compressUpdates;
};

KisSpinboxColorSelector::KisSpinboxColorSelector(QWidget *parent) : QWidget(parent) , m_d(new Private)
{

    QFormLayout *layout = new QFormLayout();
    this->setLayout(layout);

    m_d->compressUpdates  = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_d->compressUpdates, SIGNAL(timeout()), this, SLOT(slotFinishUpdate()));
}
KisSpinboxColorSelector::~KisSpinboxColorSelector()
{

}

void KisSpinboxColorSelector::slotSetColor(KoColor color)
{
    m_d->color = color;
}

void KisSpinboxColorSelector::slotSetColorSpace(const KoColorSpace *cs)
{
    if (cs == m_d->cs) {
        return;
    }

    m_d->cs = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());

    //remake spinboxes
    Q_FOREACH (QSpinBox *input, m_d->spinBoxList) {
        this->layout()->removeWidget(input);
        delete input;
    }
    m_d->spinBoxList.clear();
    Q_FOREACH (QDoubleSpinBox *input, m_d->doubleSpinBoxList) {
        this->layout()->removeWidget(input);
        delete input;
    }
    m_d->doubleSpinBoxList.clear();

    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());
    Q_FOREACH (KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            switch (channel->channelValueType()) {
            case KoChannelInfo::UINT8: {
                QSpinBox *input = new QSpinBox(this);
                input->setMinimum(0);
                input->setMaximum(0xFF);
                m_d->spinBoxList.append(input);
                this->layout()->addWidget(input);
                if (input) {
                    connect(input, SIGNAL(updated()), this,  SLOT(updateFromSpinBoxes()));
                }
            }
            break;
            case KoChannelInfo::UINT16: {
                QSpinBox *input = new QSpinBox(this);
                input->setMinimum(0);
                input->setMaximum(0xFFFF);
                m_d->spinBoxList.append(input);
                this->layout()->addWidget(input);
                if (input) {
                    connect(input, SIGNAL(updated()), this,  SLOT(updateFromSpinBoxes()));
                }
            }
            break;
            case KoChannelInfo::UINT32: {
                QSpinBox *input = new QSpinBox(this);
                input->setMinimum(0);
                input->setMaximum(0xFFFFFFFF);
                m_d->spinBoxList.append(input);
                this->layout()->addWidget(input);
                if (input) {
                    connect(input, SIGNAL(updated()), this,  SLOT(updateFromSpinBoxes()));
                }
            }
            break;
            case KoChannelInfo::FLOAT16:
            case KoChannelInfo::FLOAT32: {
                QDoubleSpinBox *input = new QDoubleSpinBox(this);
                input->setMinimum(0);
                input->setMaximum(1.0);
                m_d->doubleSpinBoxList.append(input);
                this->layout()->addWidget(input);
                if (input) {
                    connect(input, SIGNAL(updated()), this,  SLOT(updateFromSpinBoxes()));
                }
            }
            break;
            default:
                Q_ASSERT(false);
            }

        }
    }
}

void KisSpinboxColorSelector::createColorFromSpinboxValues()
{
    KoColor newColor;
    int channelcount = m_d->cs->channelCount();
    quint8 *data;
    QVector <float> channelValues(channelcount);
    channelValues.fill(1.0);
    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());

    for (int i=0; i<channelcount; i++) {
        if (m_d->spinBoxList.at(i)) {
            if (channels.at(i)->channelValueType()==KoChannelInfo::UINT8){
                channelValues[i] = KoColorSpaceMaths<quint8,float>::scaleToA(m_d->spinBoxList.at(i)->value());
            } else if (channels.at(i)->channelValueType()==KoChannelInfo::UINT16){
                channelValues[i] = KoColorSpaceMaths<quint16,float>::scaleToA(m_d->spinBoxList.at(i)->value());
            }
        } else if (m_d->doubleSpinBoxList.at(i)){
            channelValues[i] = m_d->doubleSpinBoxList.at(i)->value();
        }
    }

    m_d->cs->fromNormalisedChannelsValue(data, channelValues);
    newColor.setColor(data, m_d->cs);
    newColor.setOpacity(m_d->color.opacityU8());

    m_d->color = newColor;
}

void KisSpinboxColorSelector::slotUpdateFromSpinBoxes()
{
    m_d->allowUpdates = false;
    createColorFromSpinboxValues();
    emit sigNewColor(m_d->color);
    m_d->compressUpdates->start();
}

void KisSpinboxColorSelector::slotFinishUpdate()
{
    m_d->allowUpdates = true;
}

void KisSpinboxColorSelector::updateSpinboxesWithNewValues()
{
    int channelcount = m_d->cs->channelCount();
    QVector <float> channelValues(channelcount);
    channelValues.fill(1.0);
    m_d->cs->normalisedChannelsValue(m_d->color.data(), channelValues);
    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());

    for (int i=0; i<channelcount; i++) {
        if (m_d->spinBoxList.at(i)) {
            if (channels.at(i)->channelValueType() == KoChannelInfo::UINT8) {
                m_d->spinBoxList.at(i)->setValue(KoColorSpaceMaths<float, quint8>::scaleToA(channelValues[i]));
            } else if (channels.at(i)->channelValueType() == KoChannelInfo::UINT16) {
                m_d->spinBoxList.at(i)->setValue(KoColorSpaceMaths<float, quint16>::scaleToA(channelValues[i]));
            }
        } else if (m_d->doubleSpinBoxList.at(i)) {
            m_d->doubleSpinBoxList.at(i)->setValue(channelValues[i]);
        }
    }
}


