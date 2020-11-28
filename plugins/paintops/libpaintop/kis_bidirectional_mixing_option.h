/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2008 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_BIDIRECTIONAL_MIXING_OPTION_H
#define KIS_BIDIRECTIONAL_MIXING_OPTION_H

#include "kis_paintop_option.h"
#include <kis_types.h>
#include <kritapaintop_export.h>

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

    void readOptionSetting(const KisPropertiesConfigurationSP setting);

private:
    bool m_mixingEnabled;
};

#endif
