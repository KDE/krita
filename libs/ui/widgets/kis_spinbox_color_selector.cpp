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
#include <QLabel>
#include "kis_double_parse_spin_box.h"
#include "kis_int_parse_spin_box.h"
#include "kis_signal_compressor.h"

#include "KoChannelInfo.h"
#include "KoColorSpaceTraits.h"
#include "KoColorSpaceMaths.h"
#include "KoColorSpaceRegistry.h"

struct KisSpinboxColorSelector::Private
{
    QList <KisIntParseSpinBox*> spinBoxList;
    QList <KisDoubleParseSpinBox*> doubleSpinBoxList;
    KoColor color;
    const KoColorSpace *cs {0};
    bool chooseAlpha = false;
};

KisSpinboxColorSelector::KisSpinboxColorSelector(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    KoColor color = KoColor();
    m_d->color = color;
    slotSetColorSpace(m_d->color.colorSpace());
}
KisSpinboxColorSelector::~KisSpinboxColorSelector()
{

}

void KisSpinboxColorSelector::slotSetColor(KoColor color)
{
    m_d->color = color;
    if (m_d->color.colorSpace() != m_d->cs) {
        slotSetColorSpace(m_d->color.colorSpace());
    }
    updateSpinboxesWithNewValues();
}

void KisSpinboxColorSelector::slotSetColorSpace(const KoColorSpace *cs)
{
    if (cs == m_d->cs) {
        return;
    }

    m_d->cs = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());

    //remake spinboxes
    if (this->layout()) {
        qDeleteAll(this->children());
    }
    m_d->spinBoxList.clear();
    m_d->doubleSpinBoxList.clear();

    QFormLayout *layout = new QFormLayout(this);
    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());
    Q_FOREACH (KoChannelInfo* channel, channels) {
        QString inputLabel = channel->name();
        switch (channel->channelValueType()) {
        case KoChannelInfo::UINT8: {
            KisIntParseSpinBox *input = new KisIntParseSpinBox(this);
            input->setMinimum(0);
            input->setMaximum(0xFF);
            m_d->spinBoxList.append(input);
            layout->addRow(inputLabel,input);
            if (input) {
                connect(input, SIGNAL(valueChanged(int)), this,  SLOT(slotUpdateFromSpinBoxes()));
            }
            if (channel->channelType()==KoChannelInfo::ALPHA && m_d->chooseAlpha == false) {
                input->setEnabled(false);
                input->blockSignals(true);
            }
        }
            break;
        case KoChannelInfo::UINT16: {
            KisIntParseSpinBox *input = new KisIntParseSpinBox(this);
            input->setMinimum(0);
            input->setMaximum(0xFFFF);
            m_d->spinBoxList.append(input);
            layout->addRow(inputLabel,input);
            if (input) {
                connect(input, SIGNAL(valueChanged(int)), this,  SLOT(slotUpdateFromSpinBoxes()));
            }
            if (channel->channelType()==KoChannelInfo::ALPHA && m_d->chooseAlpha == false) {
                input->setEnabled(false);
                input->blockSignals(true);
            }
        }
            break;
        case KoChannelInfo::UINT32: {
            KisIntParseSpinBox *input = new KisIntParseSpinBox(this);
            input->setMinimum(0);
            input->setMaximum(0xFFFFFFFF);
            m_d->spinBoxList.append(input);
            layout->addRow(inputLabel,input);
            if (input) {
                connect(input, SIGNAL(valueChanged(int)), this,  SLOT(slotUpdateFromSpinBoxes()));
            }
            if (channel->channelType()==KoChannelInfo::ALPHA && m_d->chooseAlpha == false) {
                input->setEnabled(false);
                input->blockSignals(true);
            }
        }
            break;
        case KoChannelInfo::FLOAT16:
        case KoChannelInfo::FLOAT32: {
            KisDoubleParseSpinBox *input = new KisDoubleParseSpinBox(this);
            input->setMinimum(0);
            input->setMaximum(KoColorSpaceMathsTraits<float>::max);
            input->setSingleStep(0.1);
            m_d->doubleSpinBoxList.append(input);
            qDebug()<<"add "<<channel->name()<<"doubleSpinBoxList "<<m_d->doubleSpinBoxList.size();
            layout->addRow(inputLabel,input);
            if (input) {
                connect(input, SIGNAL(valueChanged(double)), this,  SLOT(slotUpdateFromSpinBoxes()));
            }
            if (channel->channelType()==KoChannelInfo::ALPHA && m_d->chooseAlpha == false) {
                input->setEnabled(false);
                input->blockSignals(true);
            }
        }
            break;
        default:
            Q_ASSERT(false);
        }

    }
    this->setLayout(layout);
}

void KisSpinboxColorSelector::createColorFromSpinboxValues()
{
    KoColor newColor;
    int channelcount = m_d->cs->channelCount();
    quint8 *data = new quint8[m_d->cs->pixelSize()];
    QVector <float> channelValues(channelcount);
    channelValues.fill(1.0);
    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());

    for (int i = 0; i < (int)qAbs(m_d->cs->colorChannelCount()); i++) {
        int channelposition = KoChannelInfo::displayPositionToChannelIndex(i, m_d->cs->channels());

        if (channels.at(i)->channelValueType()==KoChannelInfo::UINT8 && m_d->spinBoxList.at(i)){

            int value = m_d->spinBoxList.at(i)->value();
            channelValues[channelposition] = KoColorSpaceMaths<quint8,float>::scaleToA(value);

        } else if (channels.at(i)->channelValueType()==KoChannelInfo::UINT16 && m_d->spinBoxList.at(i)){

            channelValues[channelposition] = KoColorSpaceMaths<quint16,float>::scaleToA(m_d->spinBoxList.at(i)->value());

        } else if ((channels.at(i)->channelValueType()==KoChannelInfo::FLOAT16 ||
                    channels.at(i)->channelValueType()==KoChannelInfo::FLOAT32 ||
                    channels.at(i)->channelValueType()==KoChannelInfo::FLOAT64) && m_d->doubleSpinBoxList.at(i)) {

            channelValues[channelposition] = m_d->doubleSpinBoxList.at(i)->value();

        }
    }

    m_d->cs->fromNormalisedChannelsValue(data, channelValues);
    newColor.setColor(data, m_d->cs);
    newColor.setOpacity(m_d->color.opacityU8());

    m_d->color = newColor;
}

void KisSpinboxColorSelector::slotUpdateFromSpinBoxes()
{
    createColorFromSpinboxValues();
    emit sigNewColor(m_d->color);
}

void KisSpinboxColorSelector::updateSpinboxesWithNewValues()
{
    int channelcount = m_d->cs->channelCount();
    QVector <float> channelValues(channelcount);
    channelValues.fill(1.0);
    m_d->cs->normalisedChannelsValue(m_d->color.data(), channelValues);
    QList<KoChannelInfo *> channels = KoChannelInfo::displayOrderSorted(m_d->cs->channels());

    int i;
    /*while (QLayoutItem *item = this->layout()->takeAt(0))
    {
        item->widget()->blockSignals(true);
    }*/
    for (i=0; i<m_d->spinBoxList.size(); i++) {
        m_d->spinBoxList.at(i)->blockSignals(true);
    }
    for (i=0; i<m_d->doubleSpinBoxList.size(); i++) {
        m_d->doubleSpinBoxList.at(i)->blockSignals(true);
    }

    for (i = 0; i < (int)qAbs(m_d->cs->colorChannelCount()); i++) {
        int channelposition = KoChannelInfo::displayPositionToChannelIndex(i, m_d->cs->channels());
        if (channels.at(i)->channelValueType() == KoChannelInfo::UINT8 && m_d->spinBoxList.at(i)) {
            int value = KoColorSpaceMaths<float, quint8>::scaleToA(channelValues[channelposition]);
            m_d->spinBoxList.at(i)->setValue(value);
        } else if (channels.at(i)->channelValueType() == KoChannelInfo::UINT16 && m_d->spinBoxList.at(i)) {
            m_d->spinBoxList.at(i)->setValue(KoColorSpaceMaths<float, quint16>::scaleToA(channelValues[channelposition]));
        } else if ((channels.at(i)->channelValueType()==KoChannelInfo::FLOAT16 ||
                    channels.at(i)->channelValueType()==KoChannelInfo::FLOAT32 ||
                    channels.at(i)->channelValueType()==KoChannelInfo::FLOAT64) && m_d->doubleSpinBoxList.at(i)) {
            m_d->doubleSpinBoxList.at(i)->setValue(channelValues[channelposition]);
        }
    }

    for (i=0; i<m_d->spinBoxList.size(); i++) {
        m_d->spinBoxList.at(i)->blockSignals(false);
    }
    for (i=0; i<m_d->doubleSpinBoxList.size(); i++) {
        m_d->doubleSpinBoxList.at(i)->blockSignals(false);
    }
    /*while (QLayoutItem *item = this->layout()->takeAt(0))
    {
        item->widget()->blockSignals(false);
    }*/
}


