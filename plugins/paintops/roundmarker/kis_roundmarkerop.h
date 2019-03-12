/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef _KIS_ROUNDMARKEROP_H_
#define _KIS_ROUNDMARKEROP_H_

#include <QRect>

#include <kis_paintop.h>
#include <kis_types.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_spacing_option.h>
#include "kis_roundmarker_option.h"

class QPointF;
class KisPaintOpSettings;
class KisPainter;

class KisRoundMarkerOp: public KisPaintOp
{
public:
    KisRoundMarkerOp(KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image);
    ~KisRoundMarkerOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal diameter) const;

private:
    bool                      m_firstRun;
    KisPaintDeviceSP          m_tempDev;
    KisPressureSizeOption     m_sizeOption;
    KisPressureSpacingOption  m_spacingOption;
    QPointF                   m_lastPaintPos;
    qreal                     m_lastRadius;
    RoundMarkerOption         m_markerOption;
};

#endif // _KIS_ROUNDMARKEROP_H_
