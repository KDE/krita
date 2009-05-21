/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Emanuele Tamponi <emanuele@valinor.it>
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
#include "kis_bidirectional_mixing_option.h"
#include <klocale.h>

#include <QLabel>
#include <QVector>
#include <QRect>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoColor.h>

#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>


KisBidirectionalMixingOption::KisBidirectionalMixingOption()
        : KisPaintOpOption(i18n("Mixing"), false)
{
    m_checkable = true;
    m_optionWidget = new QLabel(i18n("The mixing option mixes the paint on the brush with that on the canvas."));
    m_optionWidget->hide();
    setConfigurationPage(m_optionWidget);
}


KisBidirectionalMixingOption::~KisBidirectionalMixingOption()
{
}

void KisBidirectionalMixingOption::apply(KisPaintDeviceSP dab, KisPaintDeviceSP device, KisPainter* painter, qint32 sx, qint32 sy, qint32 sw, qint32 sh, quint8 pressure, const QRect& dstRect)
{
    if (!isChecked()) return;

    KoColorSpace *cs = dab->colorSpace();
    KisPaintDeviceSP canvas = new KisPaintDevice(cs);
    KisPainter p(canvas);
    p.setCompositeOp(COMPOSITE_COPY);
    p.bitBlt(sx, sy, device, dstRect.x(), dstRect.y(), sw, sh);

    int count = cs->channelCount();
    KisRectIterator cit = canvas->createRectIterator(sx, sy, sw, sh);
    KisRectIterator dit = dab->createRectIterator(sx, sy, sw, sh);
    QVector<float> cc(count), dc(count);
    while (!cit.isDone()) {
        if (cs->alpha(dit.rawData()) > 10 && cs->alpha(cit.rawData()) > 10) {
            cs->normalisedChannelsValue(cit.rawData(), cc);
            cs->normalisedChannelsValue(dit.rawData(), dc);
            for (int i = 0; i < count; i++)
                dc[i] = (1.0 - 0.4 * pressure) * cc[i] + 0.4 * pressure * dc[i];
            cs->fromNormalisedChannelsValue(dit.rawData(), dc);
            if (dit.x() == (int)(sw / 2) && dit.y() == (int)(sh / 2))
                painter->setPaintColor(KoColor(dit.rawData(), cs));
        }
        ++cit;
        ++dit;
    }
}

void KisBidirectionalMixingOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty( "BidirectionalMixing", isChecked() );
}

void KisBidirectionalMixingOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    setChecked( setting->getBool( "BidirectionalMixing", false ) );
}

