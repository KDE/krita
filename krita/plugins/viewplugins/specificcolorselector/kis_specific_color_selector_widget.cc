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

#include "kis_specific_color_selector_widget.h"
#include <QLabel>
#include <QVBoxLayout>

#include <klocale.h>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_color_input.h"
#include <KoColorProfile.h>
#include "kis_debug.h"


KisSpecificColorSelectorWidget::KisSpecificColorSelectorWidget(QWidget* parent) : QWidget(parent), m_colorSpace(0)
{
    m_layout = new QVBoxLayout(this);
    setColorSpace(KoColorSpaceRegistry::instance()->rgb8());
    m_updateAllowed = true;
}

KisSpecificColorSelectorWidget::~KisSpecificColorSelectorWidget()
{
}

void KisSpecificColorSelectorWidget::setColorSpace(const KoColorSpace* cs)
{
    Q_ASSERT(cs);
    if (m_colorSpace && *m_colorSpace == *cs) return;
    dbgPlugins << cs->id() << " " << cs->profile()->name();
    m_colorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->id(), cs->profile());
    Q_ASSERT(m_colorSpace);
    Q_ASSERT(*m_colorSpace == *cs);
    m_color = KoColor(m_color, m_colorSpace);
    foreach(KisColorInput* input, m_inputs) {
        delete input;
    }
    m_inputs.clear();
    QList<KoChannelInfo*> channels = m_colorSpace->channels();
    foreach(KoChannelInfo* channel, channels) {
        if (channel->channelType() == KoChannelInfo::COLOR) {
            KisColorInput* input = 0;
            switch (channel->channelValueType()) {
            case KoChannelInfo::UINT8:
            case KoChannelInfo::UINT16:
            case KoChannelInfo::UINT32: {
                input = new KisIntegerColorInput(this, channel, &m_color);
            }
            break;
            case KoChannelInfo::FLOAT16:
            case KoChannelInfo::FLOAT32: {
                input = new KisFloatColorInput(this, channel, &m_color);
            }
            break;
            default:
                Q_ASSERT(false);
                input = 0;
            }
            if (input) {
                connect(input, SIGNAL(updated()), this, SLOT(update()));
                connect(this, SIGNAL(updated()), input, SLOT(update()));
                m_inputs.append(input);
                m_layout->addWidget(input);
            }
        }
    }
}

void KisSpecificColorSelectorWidget::update()
{
    if (m_updateAllowed)
        emit(colorChanged(m_color));
}

void KisSpecificColorSelectorWidget::setColor(const KoColor& c)
{
    m_updateAllowed = false;
    m_color.fromKoColor(c);
    emit(updated());
    m_updateAllowed = true;
}
