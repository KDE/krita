/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include <config-openexr.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <QHBoxLayout>
#include <QLabel>

#include <klocale.h>
#include <knuminput.h>

#include <KoChannelInfo.h>
#include <KoColor.h>

KisColorInput::KisColorInput(QWidget* parent, const KoChannelInfo* channelInfo, KoColor* color) : QWidget(parent), m_channelInfo(channelInfo), m_color(color)
{
}

void KisColorInput::init()
{
    m_layout = new QHBoxLayout(this);
    m_label = new QLabel(i18n("%1:", m_channelInfo->name()), this);
    m_layout->addWidget(m_label);
    m_input = createInput();
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
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        m_intNumInput->setValue(*(reinterpret_cast<quint8*>(data)));
        break;
    case KoChannelInfo::UINT16:
        m_intNumInput->setValue(*(reinterpret_cast<quint16*>(data)));
        break;
    case KoChannelInfo::UINT32:
        m_intNumInput->setValue(*(reinterpret_cast<quint32*>(data)));
        break;
    default:
        Q_ASSERT(false);
    }
}

KNumInput* KisIntegerColorInput::createInput()
{
    m_intNumInput = new KIntNumInput(this);
    m_intNumInput->setMinimum(0);
    switch (m_channelInfo->channelValueType()) {
    case KoChannelInfo::UINT8:
        m_intNumInput->setMaximum(0xFF);
        break;
    case KoChannelInfo::UINT16:
        m_intNumInput->setMaximum(0xFFFF);
        break;
    case KoChannelInfo::UINT32:
        m_intNumInput->setMaximum(0xFFFFFFFF);
        break;
    default:
        Q_ASSERT(false);
    }
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
        *(reinterpret_cast<double*>(data)) = v;
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}

KNumInput* KisFloatColorInput::createInput()
{
    m_dblNumInput = new KDoubleNumInput(this);
    m_dblNumInput->setMinimum(0);
    m_dblNumInput->setMaximum(1.0);
    connect(m_dblNumInput, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    return m_dblNumInput;
}

void KisFloatColorInput::update()
{
    quint8* data = m_color->data() + m_channelInfo->pos();
    switch (m_channelInfo->channelValueType()) {
#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        m_dblNumInput->setValue(*(reinterpret_cast<half*>(data)));
        break;
#endif
    case KoChannelInfo::FLOAT32:
        m_dblNumInput->setValue(*(reinterpret_cast<double*>(data)));
        break;
    default:
        Q_ASSERT(false);
    }
}

#include "kis_color_input.moc"
