/* This file is part of the KDE project
 * Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2008 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_BIDIRECTIONAL_MIXING_OPTION_H
#define KIS_BIDIRECTIONAL_MIXING_OPTION_H

#include "kis_paintop_option.h"
#include <kis_types.h>

class KisPropertiesConfiguration;
class KisPainter;
class QRect;

/**
 * The bidirectional mixing option uses the painterly framework to
 * implement bidirectional paint mixing (that is, paint on the canvas
 * dirties the brush, and the brush mixes its color with that on the
 * canvas.
 *
 * Taken from the complex paintop
 */
class PAINTOP_EXPORT KisBidirectionalMixingOption
{
public:
    KisBidirectionalMixingOption();

    ~KisBidirectionalMixingOption();

    void apply(KisPaintDeviceSP dab, KisPaintDeviceSP device, KisPainter* painter, qint32 sx, qint32 sy, qint32 sw, qint32 sh, quint8 pressure, const QRect& dstRect);
    void applyFixed(KisFixedPaintDeviceSP dab, KisPaintDeviceSP device, KisPainter* painter, qint32 sx, qint32 sy, qint32 sw, qint32 sh, quint8 pressure, const QRect& dstRect);

    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    bool m_mixingEnabled;
};

#endif
