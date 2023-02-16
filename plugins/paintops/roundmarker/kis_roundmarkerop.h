/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ROUNDMARKEROP_H_
#define _KIS_ROUNDMARKEROP_H_

#include <QRect>

#include <kis_paintop.h>
#include <kis_types.h>
#include <KisStandardOptions.h>
#include <KisSpacingOption.h>
#include "KisRoundMarkerOpOptionData.h"


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
    KisSizeOption             m_sizeOption;
    KisSpacingOption          m_spacingOption;
    QPointF                   m_lastPaintPos;
    qreal                     m_lastRadius;
    KisRoundMarkerOpOptionData   m_markerOption;
};

#endif // _KIS_ROUNDMARKEROP_H_
