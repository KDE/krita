/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_input.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <kis_debug.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <klocale.h>
#include <knuminput.h>

#include <KoChannelInfo.h>
#include <KoColor.h>
#include <KoColorSlider.h>
#include <KoColorSpace.h>

KisColorInput::KisColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color) : QWidget(parent), m_channelInfo(channelInfo), m_color(color)
{
}

void KisColorInput::init()
{
    QHBoxLayout* m_layout = new QHBoxLayout(this);
    QLabel* m_label = new QLabel(i18n("%1:", m_channelInfo->name()), this);
    m_label->setMinimumWidth(50);
    m_layout->addWidget(m_label);

    m_colorSlider = new KoColorSlider(Qt::Horizontal, this);
    m_colorSlider->setMaximumHeight(20);
    m_colorSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_layout->addWidget(m_colorSlider);

    QWidget* m_input = createInput();
    m_input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_layout->addWidget(m_input);
}

KisIntegerColorInput::KisIntegerColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color) : KisColorInput(parent, channelInfo, color)
{
    init();
}

void KisIntegerColorInput::setValue(int v)
{
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        *(reinterpret_cast<quint8*>(data)) = v;
        break;
    case KoChannelInfo::UINT16:
        *(reinterpret_cast<quint16*>(data)) = v;
        break;
    case KoChannelInfo::UINT32:
        *(reinterpret_cast<quint32*>(data)) = v;
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}

void KisIntegerColorInput::update()
{
    KoColor min = *m_color;
    KoColor max = *m_color;
    quint8* data = m_color->data() + m_channelInfo->pos();
    quint8* dataMin = min.data() + m_channelInfo->pos();
    quint8* dataMax = max.data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        m_intNumInput->setValue(*(reinterpret_cast<quint8*>(data)));
        m_colorSlider->setValue(*(reinterpret_cast<quint8*>(data)));
        *(reinterpret_cast<quint8*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint8*>(dataMax)) = 0xFF;
        break;
    case KoChannelInfo::UINT16:
        m_intNumInput->setValue(*(reinterpret_cast<quint16*>(data)));
        m_colorSlider->setValue(*(reinterpret_cast<quint16*>(data)));
        *(reinterpret_cast<quint16*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint16*>(dataMax)) = 0xFFFF;
        break;
    case KoChannelInfo::UINT32:
        m_intNumInput->setValue(*(reinterpret_cast<quint32*>(data)));
        m_colorSlider->setValue(*(reinterpret_cast<quint32*>(data)));
        *(reinterpret_cast<quint32*>(dataMin)) = 0x0;
        *(reinterpret_cast<quint32*>(dataMax)) = 0xFFFFFFFF;
        break;
    default:
        Q_ASSERT(false);
    }
    m_colorSlider->setColors(min, max);
}

QWidget* KisIntegerColorInput::createInput()
{
    m_intNumInput = new QSpinBox(this);
    m_intNumInput->setMinimum(0);
    m_colorSlider->setMaximum(0);
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        m_intNumInput->setMaximum(0xFF);
        m_colorSlider->setMaximum(0xFF);
        break;
    case KoChannelInfo::UINT16:
        m_intNumInput->setMaximum(0xFFFF);
        m_colorSlider->setMaximum(0xFFFF);
        break;
    case KoChannelInfo::UINT32:
        m_intNumInput->setMaximum(0xFFFFFFFF);
        m_colorSlider->setMaximum(0xFFFFFFFF);
        break;
    default:
        Q_ASSERT(false);
    }
    connect(m_colorSlider, SIGNAL(valueChanged(int)), m_intNumInput, SLOT(setValue(int)));
    connect(m_intNumInput, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    return m_intNumInput;
}


KisFloatColorInput::KisFloatColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color) : KisColorInput(parent, channelInfo, color)
{
    init();
}

void KisFloatColorInput::setValue(double v)
{
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        *(reinterpret_cast<half*>(data)) = v;
        break;
#endif
    case KoChannelInfo::FLOAT32:
        *(reinterpret_cast<float*>(data)) = v;
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}

QWidget* KisFloatColorInput::createInput()
{
    m_dblNumInput = new KDoubleNumInput(this);
    m_dblNumInput->setMinimum(0);
    m_dblNumInput->setMaximum(1.0);
    connect(m_colorSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(m_dblNumInput, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    return m_dblNumInput;
}

void KisFloatColorInput::sliderChanged(int i)
{
    m_dblNumInput->setValue(i / 255.0);
}

void KisFloatColorInput::update()
{
    KoColor min = *m_color;
    KoColor max = *m_color;
    quint8* data = m_color->data() + m_channelInfo->pos();
    quint8* dataMin = min.data() + m_channelInfo->pos();
    quint8* dataMax = max.data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        m_dblNumInput->setValue(*(reinterpret_cast<half*>(data)));
        m_colorSlider->setValue(*(reinterpret_cast<half*>(data)) * 255);
        *(reinterpret_cast<half*>(dataMin)) = 0.0;
        *(reinterpret_cast<half*>(dataMax)) = 1.0;
        break;
#endif
    case KoChannelInfo::FLOAT32:
        m_dblNumInput->setValue(*(reinterpret_cast<float*>(data)));
        m_colorSlider->setValue(*(reinterpret_cast<float*>(data)) * 255);
        *(reinterpret_cast<float*>(dataMin)) = 0.0;
        *(reinterpret_cast<float*>(dataMax)) = 1.0;
        break;
    default:
        Q_ASSERT(false);
    }
    m_colorSlider->setColors(min, max);
}

KisHexColorInput::KisHexColorInput(QWidget* parent, KoColor* color) : KisColorInput(parent, 0, color)
{
    QHBoxLayout* m_layout = new QHBoxLayout(this);
    QLabel* m_label = new QLabel(i18n("Color name:"), this);
    m_label->setMinimumWidth(50);
    m_layout->addWidget(m_label);

    QWidget* m_input = createInput();
    m_input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_layout->addWidget(m_input);
}

void KisHexColorInput::setValue()
{
    QString valueString = m_hexInput->text();
    valueString.remove(QChar('#'));

    QList<KoChannelInfo*> channels = m_color->colorSpace()->channels();
    channels = KoChannelInfo::displayOrderSorted(channels);
    foreach(KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            Q_ASSERT(channel->channelValueType() == KoChannelInfo::UINT8);
            quint8* data = m_color->data() + channel->pos();

            int value = valueString.left(2).toInt(0, 16);
            *(reinterpret_cast<quint8*>(data)) = value;
            valueString.remove(0, 2);
        }
    }
    emit(updated());
}

void KisHexColorInput::update()
{
    QString hexString("#");

    QList<KoChannelInfo*> channels = m_color->colorSpace()->channels();
    channels = KoChannelInfo::displayOrderSorted(channels);
    foreach(KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            Q_ASSERT(channel->channelValueType() == KoChannelInfo::UINT8);
            quint8* data = m_color->data() + channel->pos();
            hexString.append(QString("%1").arg(*(reinterpret_cast<quint8*>(data)), 2, 16, QChar('0')));
        }
    }
    m_hexInput->setText(hexString);
}

QWidget* KisHexColorInput::createInput()
{
    m_hexInput = new QLineEdit(this);

    int digits = 2*m_color->colorSpace()->colorChannelCount();
    QString pattern = QString("#?[a-fA-F0-9]{%1,%2}").arg(digits).arg(digits);
    m_hexInput->setValidator(new QRegExpValidator(QRegExp(pattern), this));
    connect(m_hexInput, SIGNAL(editingFinished()), this, SLOT(setValue()));
    return m_hexInput;
}

#include "kis_color_input.moc"
